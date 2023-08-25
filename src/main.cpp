/**
 * @file main.cpp
 * @author songhaikun
*/
#include "ace/OS_NS_errno.h"
#include "ace/OS_NS_sys_time.h"
#include "ace/os_include/os_netdb.h"
#include "ace/Log_Msg.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/Thread_Manager.h"
#include "ace/TP_Reactor.h"
#include "ace/Reactor.h"
#include "ace/OS.h"
#include "ace/Acceptor.h"

#include "data_handler.h"
#include "command_handler.h"
#include "constants.h"


using namespace MyFtpServer;
typedef ACE_Acceptor<Command_Handler, ACE_SOCK_ACCEPTOR>  Ftp_Acceptor;
int ACE_TMAIN (int, ACE_TCHAR *[]) {
    ACE_INET_Addr port_to_listen(SERVER_PORT_NUM);
    Ftp_Acceptor acceptor;
    if (acceptor.open (port_to_listen,  ACE_Reactor::instance (),  ACE_NONBLOCK) == -1)
    {
        return -1;
    }
    ACE_Reactor::instance ()->run_reactor_event_loop();
    return 0;
}



