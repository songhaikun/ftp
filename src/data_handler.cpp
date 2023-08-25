/**
 * @file data_handler.cpp
 * @author songhaikun
*/
#include "ace/OS_NS_errno.h"
#include "ace/OS_NS_sys_time.h"
#include "ace/os_include/os_netdb.h"


#include "ace/Log_Msg.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Connector.h"
#include "ace/Reactor.h"
#include "ace/Acceptor.h"
#include "data_handler.h"

#include "ace/Log_Msg.h"
#include "ace/FILE_Connector.h"
#include <iostream>
#include <grp.h>
#include <sys/file.h>
#include <sys/sendfile.h>

namespace MyFtpServer{
    Data_Handler::Data_Handler (const std::string &ip_addr, const int &port, const int &type) :
        type_ (type),
        mode_ (Data_Modes::STREAM),
        is_lock_ (false),
        wfile_try_connection_ (nullptr)
    {
        int set_addr_res = client_addr_.set (port, ip_addr.c_str ());
        if (set_addr_res == -1)
        {
            ACE_DEBUG ((LM_DEBUG, "set client address failed\n"));
        }
        memset (data_buffer_, 0, MAX_BUFFER_SIZE);
        data_link_.enable(ACE_NONBLOCK);
        file_link_.enable(ACE_NONBLOCK);
    }

    Data_Handler::~Data_Handler()
    {
        if (is_lock_ && flock(file_link_.get_handle(), LOCK_UN) != 0)
            ACE_DEBUG ( (LM_DEBUG, "shared unlock file failed\n"));
        if (is_lock_ && wfile_try_connection_ != nullptr && flock(fileno (wfile_try_connection_), LOCK_UN) != 0)
            ACE_DEBUG ( (LM_DEBUG, "exclusive unlock file failed\n"));
        is_lock_ = false;
        if (wfile_try_connection_ != nullptr)
            fclose(wfile_try_connection_);
        data_link_.close();
        file_link_.close();
        ACE_DEBUG ( (LM_DEBUG, "data connection destroyed.\n"));
    }
    int Data_Handler::data_link_init() {
        ACE_SOCK_Connector connector;
        if(connector.connect(data_link_, client_addr_) < 0) {
            ACE_DEBUG ((LM_DEBUG, "connect client's data failed\n"));
            return -1;
        }
        return 0;
    }
    int Data_Handler::file_link_init(bool is_output)
    {
        ACE_FILE_Connector connector;
        int result = 0;
        if (is_output) {
            result = connector.connect (file_link_,
                                        ACE_FILE_Addr (file_path_.c_str ()),
                                        0,
                                        ACE_Addr::sap_any,
                                        0,
                                        O_RDONLY,
                                        ACE_DEFAULT_FILE_PERMS);
        }
        else
        {
            wfile_try_connection_ = ACE_OS::fopen(file_path_.c_str (), "a");
            if (wfile_try_connection_ == nullptr)
            {
                ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("fopen error.\n")));
                return -1;
            }
        }
        if (result < 0)
        {
            ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("connect file failed.\n")));
            return -1;
        }

        return 0;
    }

    int Data_Handler::send_file()
    {
        if (type_ != Data_Types::IMAGE)
            return -1;
        
        if (!is_lock_ && flock (file_link_.get_handle (), LOCK_SH | LOCK_NB) != 0)
        {
            ACE_DEBUG ( (LM_DEBUG, "shared lock file failed\n"));
            return -1;
        }
        is_lock_ = true;
        ACE_DEBUG ( (LM_DEBUG, "lock done\n"));
        int send_count = 0;
        long offset = 0;
        while (1)
        {
            send_count = sendfile (data_link_.get_handle (), file_link_.get_handle (), &offset, 512);
            ACE_DEBUG ( (LM_DEBUG, "offset: %d\n", offset));
            if (send_count < 0)
            {
                if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK)
                    continue;
                else
                    return -1;
            } 
            else if (send_count == 0)
                return 0;
        }
        return 0;
    }

    int Data_Handler::recv_file ()
    {
        if (type_ != Data_Types::IMAGE)
            return -1;
        if (!is_lock_ && flock (fileno (wfile_try_connection_), LOCK_EX | LOCK_NB) != 0)
        {
            ACE_DEBUG ( (LM_DEBUG, "lock file failed\n"));
            return -1;
        }
        is_lock_ = true;

        ACE_FILE_Connector connector;
        connector.connect (file_link_,
                            ACE_FILE_Addr (file_path_.c_str ()),
                            0,
                            ACE_Addr::sap_any,
                            0,
                            O_RDWR | O_CREAT | O_TRUNC,
                            ACE_DEFAULT_FILE_PERMS);

        int recv_count = 0;
        while(1)
        {
            recv_count = data_link_.recv (data_buffer_, MAX_BUFFER_SIZE);
            if (recv_count < 0)
            {
                if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK)
                    continue;
                else
                    return -1;
            } 
            else if (recv_count == 0)
                return 0;
            else
            {
                int write_count = file_link_.send (data_buffer_, recv_count);
                if (write_count <= 0)
                    return -1;
            }
        }
        return 0;
    }

}