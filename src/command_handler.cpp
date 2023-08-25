/**
 * @file command_handler.cpp
 * @author songhaikun
*/
#include "ace/OS_NS_errno.h"
#include "ace/OS_NS_sys_time.h"
#include "ace/os_include/os_netdb.h"
#include "ace/Log_Msg.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/Reactor.h"
#include "ace/Acceptor.h"

#include "command_handler.h"
#include "constants.h"
#include "iostream"
namespace MyFtpServer{
    Task_Manager Command_Handler::task_mgr;

    typedef void (MyFtpServer::Command_Handler::*Handle_Func)(const MyFtpServer::cmd_info_ &cmds);

    std::unordered_map<std::string, void (Command_Handler::*)(const cmd_info_& cmds)> handle_cmd_map = {
        
        {"user", &Command_Handler::handle_user},
        {"syst", &Command_Handler::handle_syst},
        {"epsv", &Command_Handler::handle_epsv},
        {"eprt", &Command_Handler::handle_eprt},
        {"pasv", &Command_Handler::handle_pasv},
        {"feat", &Command_Handler::handle_feat},
        {"pwd" , &Command_Handler::handle_pwd},
        {"cwd" , &Command_Handler::handle_cwd},
        {"rmd" , &Command_Handler::handle_rmd},
        {"size", &Command_Handler::handle_size},
        {"list", &Command_Handler::handle_list},
        {"quit", &Command_Handler::handle_quit},
        {"mkd" , &Command_Handler::handle_mkd},
        {"type", &Command_Handler::handle_type},
        {"retr", &Command_Handler::handle_retr},
        {"stor", &Command_Handler::handle_stor},
        {"port", &Command_Handler::handle_port},
        {"dele", &Command_Handler::handle_dele},
        {"pass", &Command_Handler::handle_pass},
        
    };


    Command_Handler::Command_Handler(int use_task):
        user_info_(std::make_shared<User_Info>()), 
        is_pasv_(false), 
        pasv_port_(0),
        data_handler_(nullptr)
    {
        ACE_OS::memset(recv_buffer_, 0, sizeof(recv_buffer_));
        if(1 == use_task) {
            task_mgr.activate();
            func_to_send_response_ = &Command_Handler::send_response;
        } else {
            func_to_send_response_ = &Command_Handler::send_response_local;
        }
        user_info_->init_users_map();  //init users map
    }
    Command_Handler::~Command_Handler() {
    }

    int Command_Handler::get_real_path(std::string& para_path) {
        int len = para_path.size();
        if(0 == len) {
            para_path = user_info_->get_current_dir();
            return GET_REAL_PATH;
        }
        if('/' != para_path[len - 1]) {
            para_path += "/";
        }
        std::string old_path = user_info_->get_current_dir();
        int len_old = old_path.size();

        if('/' != old_path[len_old - 1]) old_path += "/";

        if('/' != para_path[0]) {  //相对路径
            para_path = old_path + para_path;
        }

        // 接着循环处理，遇到/.remove，遇到/..回退
        len = para_path.size();
        std::string path_t = "";
        bool parse_success = true;
        for(int i = 0; i < len; i++){
            if('/' != para_path[i]) {
                path_t += para_path[i];
            } else {
                if(i + 1 < len ) {
                    if('.' != para_path[i + 1]) {
                        path_t += para_path[i];
                    } else { // '/'+'.'
                        if(i + 2 < len) {
                            if('.' != para_path[i + 2]) {
                                if('/' == para_path[i + 2]) {  //like /./
                                    i = i + 1;
                                    continue;
                                }else {
                                    path_t += para_path[i]; //like /.abc
                                    continue;
                                }
                            } else {
                                //like /..
                                i = i + 2;
                                while(1 < path_t.size()){ //不断弹出直至'/'
                                    char c = path_t.back();
                                    path_t.pop_back();
                                    if('/' == c) {
                                        break;
                                    }
                                }
                            }
                        } else {
                            break;
                        }
                    }
                } else { // '/'为最后一个字符
                    path_t += para_path[i];
                }
            }
        }
        para_path = path_t;
        return GET_REAL_PATH;
    }

    std::string Command_Handler::get_and_clear_response_list() {
        std::string res = "";
        while(!response_list_.empty()) {
            res += response_list_.front();
            response_list_.pop_front();
        }
        return res;
    }

    int Command_Handler::send_response(const char *msg){
        char send_buf[MAX_COMMAND_BUFFER_SIZE] = {0};
        strncpy(send_buf, msg, strlen(msg));
        int send_len = this->peer().send(send_buf, strlen(send_buf));
        ACE_DEBUG ((LM_DEBUG,  ACE_TEXT ("send response: %*s"), strlen(send_buf) - 2, send_buf));
        return (send_len == strlen(send_buf)) ? 0 : 1;
    }

    int Command_Handler::send_response_local(const char *msg){
        response_list_.emplace_back(msg);
        return (strlen(msg) > 0) ? 1 : 0;
    }

    void Command_Handler::handle_user(const cmd_info_& cmds) {
        std::string username = std::string(cmds.buf + cmds.para_start);
        if(NO_SUCH_USERNAME == user_info_->check_username(username) || !user_info_->set_username(username)) {
            (this->*func_to_send_response_)(COMMAND_530_LOGIN_FAILED); //o
            return ;
        }
        (this->*func_to_send_response_)(COMMAND_331); //o
    }

    void Command_Handler::handle_pass(const cmd_info_& cmds) {
        if(0 != chdir("/home/hksong/hksong/ftp_home/")){
            return ;
        }
        std::string password = std::string(cmds.buf + cmds.para_start);
        // std::string username = user_info_->get_username();
        int res = user_info_->check_user(user_info_->get_username(), password);
        if(NO_SUCH_USERNAME == res || USER_PASSWORD_WRONG == res) {
            (this->*func_to_send_response_)(COMMAND_530_LOGIN_FAILED);
            return ;
        }
        user_info_->login();
        (this->*func_to_send_response_)(COMMAND_230);
    }

    void Command_Handler::handle_syst(const cmd_info_& cmds) {
        (this->*func_to_send_response_)(COMMAND_215);
    }

    void Command_Handler::handle_epsv(const cmd_info_& cmds) {
        CHECK_LOGGING();
        (this->*func_to_send_response_)(COMMAND_550_NOT_SUPPORT);  //TODO: need support
    }


    void Command_Handler::handle_eprt(const cmd_info_& cmds) {
        CHECK_LOGGING();
        (this->*func_to_send_response_)(COMMAND_550_NOT_SUPPORT);  //TODO: need support
    }

    void Command_Handler::handle_pasv(const cmd_info_& cmds) {
        CHECK_LOGGING();
        if(!is_pasv_) {
            ACE_INET_Addr local_addr;
            local_addr.set((u_short)0, (ACE_UINT32) INADDR_ANY);
            if(pasv_acceptor_.open(local_addr) < 0) {
                ACE_DEBUG ( (LM_DEBUG, "open pasv port failed\n"));
                (this->*func_to_send_response_)(COMMAND_500_FAILED);
                return ;
            }
            pasv_acceptor_.get_local_addr(local_addr);
            ACE_DEBUG ( (LM_DEBUG, "open port: %d\n", local_addr.get_port_number ()));
            pasv_port_ = local_addr.get_port_number();
            is_pasv_ = true;
        }
        if(data_handler_){
            data_handler_.reset();
        }
        
        data_handler_ = std::unique_ptr<Data_Handler>(new Data_Handler(data_type_));
        u_short high_byte = pasv_port_ >> 8;
        u_short low_byte = pasv_port_ & 0x00ff;
        std::stringstream pasv_address_ss;
        pasv_address_ss << HOST_ADDRESS << ',' << high_byte << ',' << low_byte;
        std::string res = COMMAND_227_BEGIN + pasv_address_ss.str() + COMMAND_227_END;
        (this->*func_to_send_response_)(res.c_str());
    }

    void Command_Handler::handle_feat(const cmd_info_& cmds) {
        CHECK_LOGGING();
        (this->*func_to_send_response_)(COMMAND_211);
    }

    void Command_Handler::handle_pwd(const cmd_info_& cmds) {
        CHECK_LOGGING();
        std::string response_pwd = COMMAND_257_PWD_BEIGN + user_info_->get_current_dir() + COMMAND_257_PWD_BEIGN_END;
        (this->*func_to_send_response_)(response_pwd.c_str());
    }
    void Command_Handler::handle_cwd(const cmd_info_& cmds) {
        CHECK_LOGGING();
        std::string new_path = std::string(cmds.buf + cmds.para_start);
        if(GET_REAL_PATH == get_real_path(new_path) && 0 == chdir(new_path.c_str()) 
            && user_info_->set_current_dir(new_path)){
            (this->*func_to_send_response_)(COMMAND_250_CWD_SUCCESS);
        }
        else{
            (this->*func_to_send_response_)(COMMAND_550_CWD_FAILED);
        }
    }
    void Command_Handler::handle_mkd(const cmd_info_& cmds) {
        CHECK_LOGGING();
        std::string new_path = std::string(cmds.buf + cmds.para_start);
        if(GET_REAL_PATH == get_real_path(new_path) && (ACE_OS::mkdir(new_path.c_str())>= 0 )) {
            //创建dir
            (this->*func_to_send_response_)(COMMAND_257_MKD_SUCCESS);
            return ;
        }
        (this->*func_to_send_response_)(COMMAND_500_FAILED);
    }

    void Command_Handler::handle_dele(const cmd_info_& cmds) {
        CHECK_LOGGING();
        std::string new_path = std::string(cmds.buf + cmds.para_start);
        struct stat buffer;
        if(GET_REAL_PATH == get_real_path(new_path))
        {
            new_path.pop_back();
            if(ACE_OS::stat(new_path.c_str(), &buffer) == 0 
               && std::remove(new_path.c_str()) == 0){
                (this->*func_to_send_response_)(COMMAND_250_DELE_SUCCESS);
                return ;
            }
        }
        (this->*func_to_send_response_)(COMMAND_500_FAILED);
    }

    void Command_Handler::handle_rmd(const cmd_info_& cmds) {
        CHECK_LOGGING();
        std::string new_path = std::string(cmds.buf + cmds.para_start);
        if(GET_REAL_PATH == get_real_path(new_path)) {
            struct stat buffer;
            if(ACE_OS::stat(new_path.c_str(), &buffer) != 0 
                            || (buffer.st_mode & S_IFMT) != S_IFDIR 
                            || ACE_OS::rmdir(new_path.c_str()) != 0) {
                (this->*func_to_send_response_)(COMMAND_550_RMD_FAILED);
                return ;
            }
            (this->*func_to_send_response_)(COMMAND_250_DELE_SUCCESS);
            return;
        }
    }

    void Command_Handler::handle_size(const cmd_info_& cmds) {
        CHECK_LOGGING();
        std::string new_path = std::string(cmds.buf + cmds.para_start);
        
        if(GET_REAL_PATH == get_real_path(new_path)) {
            new_path.pop_back();
            struct stat s{};
            if(-1 == lstat(new_path.c_str(), &s)) {
                perror("lstat");
                (this->*func_to_send_response_)(COMMAND_550_SIZE_FAILED);
                return;
            }
            std::string res = "213 " + std::to_string(s.st_size) + "\r\n";
            (this->*func_to_send_response_)(res.c_str());
        } else {
            (this->*func_to_send_response_)(COMMAND_550_SIZE_FAILED);
        }
        
    }
    void Command_Handler::list_dir(std::list<std::string>& list, const char* new_path, DIR* dir) {
        list.emplace_back(
            "文件类型    链接数 uid     gid          大小    日期       文件名\r\n");
        struct dirent *dentry;
        while((dentry = readdir(dir)) != nullptr) {
            char absolute_path[2048] = {0};
            ACE_OS::snprintf(absolute_path, sizeof(absolute_path), "%s/%s", 
                             new_path, dentry->d_name);
            struct stat s{};
            if(-1 == lstat(absolute_path, &s)) {
                perror("lstat");
                continue;
            }
            if(0 == strcmp(dentry->d_name, ".") || 0 == strcmp(dentry->d_name, "..")) {
                continue;
            }
            char perm[] = "----------";
            mode_t mode = s.st_mode & S_IFMT;
            switch (mode) {
            case S_IFREG:
                perm[0] = '-';
                break;
            case S_IFDIR:
                perm[0] = 'd';
                break;
            case S_IFLNK:
                perm[0] = 'l';
                break;
            case S_IFIFO:
                perm[0] = 'p';
                break;
            case S_IFSOCK:
                perm[0] = 's';
                break;
            case S_IFCHR:
                perm[0] = 'c';
                break;
            case S_IFBLK:
                perm[0] = 'b';
                break;
            default:
                perm[0] = '?';
                break;
            }

            if (s.st_mode & S_IRUSR) {
                perm[1] = 'r';
            }
            if (s.st_mode & S_IWUSR) {
                perm[2] = 'w';
            }
            if (s.st_mode & S_IXUSR) {
                perm[3] = 'x';
            }
            if (s.st_mode & S_IRGRP) {
                perm[4] = 'r';
            }
            if (s.st_mode & S_IWGRP) {
                perm[5] = 'w';
            }
            if (s.st_mode & S_IXGRP) {
                perm[6] = 'x';
            }
            if (s.st_mode & S_IROTH) {
                perm[7] = 'r';
            }
            if (s.st_mode & S_IWOTH) {
                perm[8] = 'w';
            }
            if (s.st_mode & S_IXOTH) {
                perm[9] = 'x';
            }
            if (s.st_mode & S_ISUID) {
                perm[3] = perm[3] == 'x' ? 's' : 'S';
            }
            if (s.st_mode & S_ISGID) {
                perm[6] = perm[6] == 'x' ? 's' : 'S';
            }
            if (s.st_mode & S_ISVTX) {
                perm[9] = perm[9] == 'x' ? 't' : 'T';
            }
            int offset = 0;
            char buf[2048] = {0};
            offset += ACE_OS::snprintf(buf, sizeof(buf), "%s ", perm);
            offset +=
                    ACE_OS::snprintf(buf + offset, sizeof(buf), "%3lu %-8d %-8d ",
                                    s.st_nlink, s.st_uid, s.st_gid);
            offset += ACE_OS::snprintf(buf + offset, sizeof(buf), "%8lu ",
                                    s.st_size);
            struct timeval tv{};
            ::gettimeofday(&tv, NULL);
            const char* format = (s.st_mtime > tv.tv_sec ||
                                  tv.tv_sec - s.st_mtime > 365 / 2 * 24 * 60 * 60)
                                    ? "%b %e %H:%M"
                                    : "%b %e  %Y";
            struct tm* tm_ptr = localtime(&tv.tv_sec);
            char date[64] = {0};
            strftime(date, sizeof(date), format, tm_ptr);
            offset += ACE_OS::snprintf(buf + offset, sizeof(buf), "%s ", date);
            if (S_ISLNK(s.st_mode)) {
                char src_path[1024] = {0};
                ssize_t ret = readlink(absolute_path, src_path, sizeof(src_path));
                if (ret == -1) {
                    perror("readlink");
                }
                ACE_OS::snprintf(buf + offset, sizeof(buf), "%s -> %s\r\n",
                                dentry->d_name, src_path);
            } else {
                ACE_OS::snprintf(buf + offset, sizeof(buf), "%s\r\n", dentry->d_name);
            }
            list.emplace_back(buf);
            // list_ans += std::string(dentry->d_name);
        }
    }
    int Command_Handler::make_data_connection ()
    {
        if(is_pasv_)
        {
            if (pasv_acceptor_.accept(data_handler_->get_data_link()) < 0)
            {
                ACE_DEBUG ( (LM_DEBUG, "pasv accept failed\n"));
                return -1;
            }
            ACE_DEBUG ( (LM_DEBUG, "pasv connection succeed.\n"));
            return 0;
        }
        else
        {
            return data_handler_->data_link_init();
        }
        
    }
    void Command_Handler::handle_list(const cmd_info_& cmds) {
        CHECK_LOGGING();
        std::string new_path = std::string(cmds.buf + cmds.para_start);
        DIR* dir = nullptr;

        if(GET_REAL_PATH_FAILED == get_real_path(new_path) || (dir = opendir(new_path.c_str())) == nullptr) {
            //TODO:send_respond
            return;
        }
        //open success
        if (make_data_connection () == -1)
        {
            (this->*func_to_send_response_)(COMMAND_425_DATA_CONN_FAILED);
            return ;
        }
        (this->*func_to_send_response_)(COMMAND_150);
        std::list<std::string> list;
        list_dir(list,new_path.c_str(), dir);
        closedir(dir);

        for(const std::string& str : list) {
            data_handler_->get_data_link().send(str.c_str(),str.size());
            data_handler_->get_data_link().send("\r\n", sizeof "\r\n");
        }

        data_handler_.reset();
        (this->*func_to_send_response_)(COMMAND_226_LIST_OK);
        return;
    }
    void Command_Handler::handle_quit(const cmd_info_& cmds) {
        user_info_->logout();
        (this->*func_to_send_response_)(COMMAND_221);
    }
    
    void Command_Handler::handle_type(const cmd_info_& cmds) {
        CHECK_LOGGING();
        std::string para = std::string(cmds.buf + cmds.para_start);
        char type = para[0];
        ACE_DEBUG ( (LM_DEBUG, "type is %c\n",type));
        switch(type) {
        case 'I':
        case 'i':
            data_type_ = Data_Types::IMAGE;
            break;
        case 'A':
        case 'a':
            data_type_ = Data_Types::ASCII;
            break;
        default:
            (this->*func_to_send_response_)(COMMAND_500_FAILED);
            return;
        }
        (this->*func_to_send_response_)(COMMAND_SUCCESS);
        return ;
    }

    void Command_Handler::handle_retr(const cmd_info_& cmds) {
        CHECK_LOGGING();
        if(!data_handler_) {
            ACE_DEBUG ((LM_DEBUG, "data link not set.\n"));
            (this->*func_to_send_response_)(COMMAND_500_FAILED);
            return ;
        }
        std::string new_path = std::string(cmds.buf + cmds.para_start);
        if(GET_REAL_PATH == get_real_path(new_path)) {
            new_path.pop_back(); //去掉'/'
            data_handler_->set_file_path (new_path);
            if (-1 == data_handler_->file_link_init (true))
            {  
                (this->*func_to_send_response_)(COMMAND_500_FAILED);
                return ;
            }
            if (-1 == make_data_connection()) {
                (this->*func_to_send_response_)(COMMAND_425_DATA_CONN_FAILED);
                return ;
            }
            (this->*func_to_send_response_)(COMMAND_150_CONNECT_READY);
            if (-1 == data_handler_->send_file())
            {
                ACE_DEBUG ( (LM_DEBUG, "send file failed\n"));
                (this->*func_to_send_response_)(COMMAND_500_FAILED);
                return ;
            }
            data_handler_.reset ();
            (this->*func_to_send_response_)(COMMAND_SUCCESS);
            //TODO time_of_last_command
            return;
        } else {
            return;
        }
        
    }

    void Command_Handler::handle_stor(const cmd_info_& cmds) {
        //获取想要下载到的路径
        //初始化数据连接
        //发送数据流
        //建立文件，并将接收到的信息填充至文件
        CHECK_LOGGING();
        std::string new_path = std::string(cmds.buf + cmds.para_start);
        if(!data_handler_ || GET_REAL_PATH_FAILED == get_real_path(new_path)) {
            ACE_DEBUG ((LM_DEBUG, "data link not set.\n"));
            (this->*func_to_send_response_)(COMMAND_500_FAILED);
            return ;
        }
        new_path.pop_back();
        data_handler_->set_file_path(new_path);
        if(data_handler_->file_link_init (false) == -1)
        {
            ACE_DEBUG ((LM_DEBUG, "file link init error.\n"));
            (this->*func_to_send_response_)(COMMAND_500_FAILED);
            data_handler_.reset();
            return ;
        }
        if (make_data_connection() == -1)
        {
            (this->*func_to_send_response_)(COMMAND_425_DATA_CONN_FAILED);
            data_handler_.reset ();
            return ;
        }
        (this->*func_to_send_response_)(COMMAND_150_CONNECT_READY);

        if (data_handler_->recv_file() == -1)
        {
            ACE_DEBUG ( (LM_DEBUG, "recv file failed\n"));
            (this->*func_to_send_response_)(COMMAND_500_FAILED);
            data_handler_.reset ();
            return ;
        }
        data_handler_.reset ();
        (this->*func_to_send_response_)(COMMAND_SUCCESS);
        return;
    }
    void Command_Handler::handle_port(const cmd_info_& cmds){
        CHECK_LOGGING();
        std::istringstream param(cmds.buf + cmds.para_start);
        int h1, h2, h3, h4, p1, p2;
        char comma;
        param >> h1 >> comma >> h2 >> comma >> h3 >> comma >> h4 
            >> comma >> p1 >> comma >> p2;
        int port = (p1 << 8) + p2;
        std::stringstream ss;
        ss << h1 << '.' << h2 << '.' << h3 << '.' << h4;
        std::string ip_addr = ss.str ();

        if(is_pasv_) {
            pasv_acceptor_.close();
            is_pasv_ = false;
        }

        if(data_handler_) {
            data_handler_.reset();
        }
        data_handler_ = std::unique_ptr<Data_Handler> (new Data_Handler(ip_addr, port, data_type_));
        (this->*func_to_send_response_)(COMMAND_SUCCESS);
        return ;
    }
    ACE_HANDLE Command_Handler::get_handle(void) const {
        return this->peer().get_handle();
    }
    int Command_Handler::open(void *p) {
        if (super::open (p) == -1) {
            return -1;
        }
        ACE_TCHAR peer_name[MAXHOSTNAMELEN];
        ACE_INET_Addr peer_addr;
        if (this->peer ().get_remote_addr (peer_addr) == 0 && peer_addr.addr_to_string (peer_name, MAXHOSTNAMELEN) == 0)
        {
            ACE_DEBUG ((LM_DEBUG,  ACE_TEXT ("Connection from %s\n"), peer_name));
            char msg[] = COMMAND_220;
            this->peer().send(msg, strlen(msg));
            ACE_DEBUG ((LM_DEBUG,  ACE_TEXT ("send response: %*s"), strlen(msg) - 2, msg));
        }
        
        return 0;
    }
    int Command_Handler::parse_line(cmd_info_& cmds) { //cmd_end 为开区间，表示命令的最后一个字符的位置+1, para_start为参数的第一个字符位置（闭区间）
        char* ptr = strchr(cmds.buf, '\r');
        if(nullptr == ptr || *(ptr + 1) != '\n') {
            cmds.len = 0, cmds.cmd_end = -1, cmds.para_start = -1, cmds.buf[0] = 0;
            return PARSE_LINE_ERROR;
        }
        *ptr = 0;
        cmds.len = ptr - cmds.buf;
        ptr = strchr(cmds.buf, ' ');
        if(nullptr != ptr) {
            cmds.cmd_end = ptr - cmds.buf;
            while(ptr < cmds.buf + cmds.len && *ptr == ' '){
                *ptr = 0;
                ptr++;
            }
            if(ptr < cmds.buf + cmds.len) {
                cmds.para_start = ptr - cmds.buf;
                return PARSE_LINE_OK_CMD_AND_PARA;
            } 
            *(cmds.buf + cmds.cmd_end) = 0;
            cmds.len = cmds.cmd_end;
        } else{
            cmds.cmd_end = cmds.len;
        }
        cmds.para_start = -1;
        return PARSE_LINE_OK_ONLY_CMD;
        
    }
    // typedef void (*Handle_Func)(const MyFtpServer::cmd_info_ &cmds);
    
    Handle_Func Command_Handler::process_cmd(const cmd_info_& cmds) {
        std::string cmd_s = std::string(cmds.buf, cmds.cmd_end);
        std::transform(cmd_s.begin(), cmd_s.end(), cmd_s.begin(), ::tolower);
        auto iter = handle_cmd_map.find(cmd_s);
        if(iter == handle_cmd_map.end()){ //未识别命令
            return nullptr;
        }
        return iter->second;
    }

    int Command_Handler::handle_input (ACE_HANDLE)
    {
        
        char buf[BUFSIZ] = {0};
        ACE_INET_Addr from_addr;
        // ACE_DEBUG((LM_DEBUG, "(%P|%t) activity occurred on handle %d!\n", this->peer().get_handle()));
        ssize_t n = this->peer().recv(buf, sizeof buf);
        if(-1 == n) { 
            ACE_ERROR((LM_ERROR, "%p/n", "handle_input"));
        } 
        else if (0 == n) {
            return 0;
        } 
        else {
            cmd_info_ cmds = {buf, 0, 0, 0};
            int result = parse_line(cmds);
            if(PARSE_LINE_ERROR == result) {
                ACE_OS::memset(buf, 0, BUFSIZ); //清空
                ACE_DEBUG((LM_ERROR, "parse line error\n"));
                return 0;
            }
            if(PARSE_LINE_OK_ONLY_CMD == result || PARSE_LINE_OK_CMD_AND_PARA == result) {
                // ACE_DEBUG((LM_DEBUG, "parse line finish, get cmd= %*s\n",cmds.len, buf));
                auto func = process_cmd(cmds);
                if(nullptr == func) {
                    (this->*func_to_send_response_)(COMMAND_550_NOT_SUPPORT);
                }
                else {
                    (this->*func)(cmds);
                }
                
            }
            // if(PARSE_LINE_OK_CMD_AND_PARA == result) {
            //     ACE_DEBUG((LM_DEBUG, "parse line finish, get cmd= %*s, para=%s\n",cmds.cmd_end, buf, buf + cmds.para_start));
            //     (this->*process_cmd(cmds))(cmds);
            // }
            
            // char msg[] = "230 Login successful.\r\n";
            // this->peer().send(msg, strlen(msg));
            // ACE_Message_Block *mb = NULL;
            // ACE_NEW_RETURN(mb, ACE_Message_Block(n, ACE_Message_Block::MB_DATA, 0, buf), -1);
            // mb->wr_ptr(n);
            // task_mgr.putq(mb);
        }
        return 0;
    }
    int Command_Handler::handle_close (ACE_HANDLE h, ACE_Reactor_Mask mask)
    {
        return super::handle_close (h, mask);
    }

}