/**
 * @file command_handler.h
 * @author songhaikun
*/
#ifndef __REQUEST_HANDLER_TCP_H_
#define __REQUEST_HANDLER_TCP_H_
#include "ace/Synch_Traits.h"
#include "ace/Null_Condition.h"
#include "ace/Null_Mutex.h"
#include "ace/Message_Block.h"
#include "ace/SOCK_Stream.h"
#include "ace/Svc_Handler.h"
#include "ace/Thread_Manager.h"

#include <utility>
#include <unordered_map>
#include <unistd.h>
#include <memory>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <list>
#include <time.h>

#include "task_manager.h"
#include "data_handler.h"
#include "user_info.h"

namespace MyFtpServer{
    /**
     * @brief 记录一次命令中的信息，包括命令与参数定位信息与命令长度
    */
    struct cmd_info_{
        char *buf;
        int cmd_end;
        int para_start;
        int len;
    };
    /**
     * @brief 处理连接请求并分发，接受控制通道信息并响应
    */
    class Command_Handler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>
    {
        typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;
        typedef std::pair<std::string, std::string> cmd_line_t;  //如 user + haikunsong
        typedef void (MyFtpServer::Command_Handler::*Handle_Func)(const MyFtpServer::cmd_info_ &cmds);
    public:
        Command_Handler(int use_task = 1);
        virtual ~Command_Handler();
        virtual ACE_HANDLE get_handle(void) const;
        /**
         * @brief 初始化控制通道连接
         * @return 连接是否成功
        */
        int open (void * = 0);

        /**
         * @brief 接收控制连接数据，解析并处理与响应数据
         * @param fd 连接句柄
         * @return 解析与处理是否成功
         * @see parse_line() process_cmd() send_response() 
        */
        virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE); 

        /**
         * @brief 处理客户端关闭连接事件
        */
        virtual int handle_close(ACE_HANDLE handle,  ACE_Reactor_Mask close_mask);

        /**
         * @brief 解析接收的报文，将报文拆分成命令+参数的形式
         * @param cmds[out] 解析到的报文信息
         * @return 解析是否成功
        */
        int parse_line(cmd_info_& cmds);

        /**
         * @brief 根据命令获取对应的具体处理函数地址
         * @param cmd 存放命令信息
         * @return 处理函数的地址
        */
        Handle_Func process_cmd(const cmd_info_& cmd);

        /**
         * @brief 处理对应的ftp命令
         * @param cmds 存放命令信息
        */
        void handle_user(const cmd_info_& cmds);
        void handle_pass(const cmd_info_& cmds);
        void handle_syst(const cmd_info_& cmds);
        void handle_epsv(const cmd_info_& cmds);
        void handle_eprt(const cmd_info_& cmds);
        void handle_feat(const cmd_info_& cmds);
        void handle_pwd(const cmd_info_& cmds);
        void handle_cwd(const cmd_info_& cmds);
        void handle_pasv(const cmd_info_& cmds);
        void handle_list(const cmd_info_& cmds);
        void handle_type(const cmd_info_& cmds);
        void handle_size(const cmd_info_& cmds);
        void handle_mkd(const cmd_info_& cmds);
        void handle_rmd(const cmd_info_& cmds);
        void handle_dele(const cmd_info_& cmds);
        void handle_stor(const cmd_info_& cmds);
        void handle_retr(const cmd_info_& cmds);
        void handle_quit(const cmd_info_& cmds);
        void handle_port(const cmd_info_& cmds);

        /**
         * @brief 建立数据通道连接（主动/被动）
         * @return 连接是否成功
        */
        int make_data_connection();

        /**
         * @brief 将参数路径与当前路径拼接得到目标路径
         * @return 拼接是否成功
        */
        int get_real_path(std::string& para_path);

        /**
         * @brief 发送响应ftp报文
         * @param para_path 路径参数，可以为绝对路径或相对路径形式
         * @return 发送是否成功
        */
        int send_response(const char *msg);

        /**
         * @brief 将响应报文加入链表（不发送）
         * @param msg 响应字符串
         * @return 是否成功
        */
        int send_response_local(const char *msg);

        /**
         * @brief 得到list命令的响应数据存入list链表中
         * @param list 按行存放得到的各个目录项信息
         * @param new_path 需要检索的目录
         * @param dir 检索目录的DIR类信息
        */
        void list_dir(std::list<std::string>& list, const char* new_path, DIR* dir);

        /**
         * @brief 将响应报文链表拼接成字符串并清空链表
         * @return 响应报文对应的字符串
        */
        std::string get_and_clear_response_list();
    private:
        /// @brief 函数指针，用于改变handle_xxx中响应发送的行为
        int (MyFtpServer::Command_Handler::*func_to_send_response_)(const char *msg);
        /// @brief 响应报文链表
        std::list<std::string> response_list_;
        /// @brief 任务管理类，扩展
        static Task_Manager task_mgr;
        /// @brief 用户信息类
        std::shared_ptr<User_Info> user_info_;
        /// @brief 是否为被动模式
        bool is_pasv_;
        /// @brief 被动模式下的客户连接端口
        u_short pasv_port_;
        /// @brief 用于被动模式
        ACE_SOCK_Acceptor pasv_acceptor_;
        /// @brief 数据通道传送数据类型
        int data_type_;
        /// @brief 数据通道控制器接口
        std::unique_ptr<Data_Handler> data_handler_;
        /// @brief 接收缓存
        char recv_buffer_[MAX_COMMAND_BUFFER_SIZE]; 
    };
}
#endif