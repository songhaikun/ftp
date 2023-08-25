/**
 * @file data_handler.h
 * @author songhaikun
*/
#ifndef __DATA_HANDLER_H__
#define __DATA_HANDLER_H__
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"
#include "ace/FILE_IO.h"

#include <string>

#include "constants.h"
namespace MyFtpServer{

    enum Data_Modes 
    {
        STREAM = 1,
        BLOCK = 2,
        COMPRESSED = 3,
    };
    /// @brief 连接类型

    enum Data_Types 
    {
        ASCII = 1,
        EBCDIC = 2,
        IMAGE = 3,
    };
    /// @brief 连接数据类型

    /**
     * @brief 数据通道连接处理类
    */
    class Data_Handler {
    public:
        Data_Handler(const int &type) : 
            mode_ (Data_Modes::STREAM),
            type_ (type),
            is_lock_ (false),
            wfile_try_connection_ (nullptr)
        {}

        /**
         * @brief 构造函数，初始化数据连接类
         * @param ip_addr 客户端ip地址
         * @param port 客户接收数据连接地址
         * @param type 连接类型
         */
        Data_Handler(const std::string &ip_addr, const int &port, const int &type);
        
        /**
         * @brief 建立主动连接
         * @return int 0 成功，1 失败
         */
        virtual int data_link_init();

        /**
         * @brief 建立本地文件数据连接
         * @param is_output 用于判断是否需要向客户输出
         * @return int 0 成功，-1 失败
         */
        virtual int file_link_init(bool is_output);

        /**
         * @brief 向客户发送数据
         * @return int 0 成功，-1 失败
         */
        virtual int send_file();

        /**
         * @brief 接收用户数据并存入本地服务器
         * @return int 0 成功，-1 失败
         */
        virtual int recv_file ();

        /// @brief 返回数据连接对象
        ACE_SOCK_Stream &get_data_link () { return data_link_; }

        /// @brief 设置文件路径
        void set_file_path(const std::string &file_path) { file_path_ = file_path; }

        /// @brief 析构函数
        virtual ~Data_Handler();
    private:
        /// @brief 数据连接对象
        ACE_SOCK_Stream data_link_;

        /// @brief 客户端ip地址
        ACE_INET_Addr client_addr_;

        /// @brief 操作的文件对象
        ACE_FILE_IO file_link_;

        /// @brief 数据缓冲
        char data_buffer_[MAX_BUFFER_SIZE];

        /// @brief 数据传送模式
        int mode_;

        /// @brief 传送数据类型
        int type_;

        /// @brief 文件地址
        std::string file_path_;

        /// @brief 文件是否加锁
        bool is_lock_;

        /// @brief 本地服务器中对应客户端传送的文件
        FILE *wfile_try_connection_;
    };
}

#endif