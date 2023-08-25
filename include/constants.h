/**
 * @file constants.h
 * @author songhaikun
*/
#ifndef __CONSTANTS_H_
#define __CONSTANTS_H_
#include "ace/OS.h"
#include <string>
#include <vector>

namespace MyFtpServer {
    #define USERS_CONFIG_PATH          "/home/hksong/hksong/prjs/myftpserver/include/users_config.txt"

    #define USERS_CONFIG_OPEN_ERROR    -1
    #define USERS_MAP_INIT_SUCCESS     1
    #define HAS_THIS_USERNAME          1
    #define HAS_THIS_USER              2
    #define USER_PASSWORD_WRONG        0
    #define NO_SUCH_USERNAME           -1

    #define PARSE_LINE_ERROR          -1
    #define PARSE_LINE_OK_ONLY_CMD     0
    #define PARSE_LINE_OK_CMD_AND_PARA 1
    #define GET_REAL_PATH              1
    #define GET_REAL_PATH_FAILED       0
    
    static const size_t SERVER_THREAD_POOL_SIZE = 5;      //进行数据接收的线程池大小
    static const size_t TASK_THREAD_POOL_SIZE   = 5;      //进行数据处理的线程池大小
    static const size_t BUFFER_SIZE             = 4096;   //数据缓冲区大小
    static const size_t SERVER_PORT_NUM         = 10101;  //服务器的通信端口号
    static const size_t MAX_COMMAND_BUFFER_SIZE = 1024;   //命令长度
    static const size_t MAX_BUFFER_SIZE         = 2048;   //数据buffer长度

    #define HOST_ADDRESS               "127,0,0,1"
    #define BASE_DIR                   "/home/hksong/"

    #define COMMAND_150 "150 Here comes the directory listing.\r\n"
    #define COMMAND_150_CONNECT_READY "150 Data connection already open; transfer starting\r\n"
    #define COMMAND_SUCCESS "200 Command successful\r\n"
    #define COMMAND_200 "200 PORT command successful.\r\n"
    #define COMMAND_200_PASV "200 PASV command successful.\r\n"                 
    #define COMMAND_211 "211-Features:\r\nPASV\r\nSIZE\r\nUTF8\r\n211 End.\r\n"                
    #define COMMAND_215 "215 Linux Ubuntu 22.04. \r\n"
    #define COMMAND_220 "220 (MyFtpServer0.0.1)\r\n"                
    #define COMMAND_221 "221 Goodbye.\r\n"                 
    #define COMMAND_226 "226 Transfer complete.\r\n" 
    #define COMMAND_226_LIST_OK "226 Directory send OK.\r\n"
    #define COMMAND_227_BEGIN "227 Entering Passive Mode. ("
    #define COMMAND_227_END ")\r\n"
    #define COMMAND_229 "229 Entering Extended Passive Mode (|||18139|)"            
    #define COMMAND_230 "230 Login successful. \r\n"                 
    #define COMMAND_250_RMD "250 Remove directory operation successful.\r\n"           
    #define COMMAND_250_CWD "250 Directory successfully changed.\r\n"        
    #define COMMAND_250_CWD_SUCCESS "250 Directory successfully changed. \r\n"     
    #define COMMAND_250_DELE "250 Delete operation successful.\r\n"          
    #define COMMAND_250_DELE_SUCCESS "250 Delete operation successful.\r\n"
    #define COMMAND_257_PWD_BEIGN "257 \""
    #define COMMAND_257_PWD_BEIGN_END "\"is the current directory.\r\n"
    #define COMMAND_257_MKD_SUCCESS "257 making directory OK\r\n"
    #define COMMAND_331 "331 Please specify the password.\r\n"                 
    #define COMMAND_425_DATA_CONN_FAILED "425 Cannot open data connection.\r\n"
    #define COMMAND_425_OPEN_FILE_FAILED "425 File opening failed.\r\n"
    #define COMMAND_500_UNKNOW "500 Unknown command. \r\n" 
    #define COMMAND_500_FAILED "500 Command failed.\r\n"          
    #define COMMAND_500_LOGIN "500 Please login first. \r\n"       
    #define COMMAND_502 "502 Unimplement command.\r\n"                 
    #define COMMAND_530_LOGIN_FAILED "530 Login incorrect.\r\n"    
    #define COMMAND_550_NOT_SUPPORT "550 command not support.\r\n"
    #define COMMAND_550_MKD_FAILED "550 Create directory operation failed.\r\n"    
    #define COMMAND_550_RMD_FAILED "550 Remove directory operation failed.\r\n"  
    #define COMMAND_550_CWD_FAILED "550 Failed to change directory.\r\n"     
    #define COMMAND_550_DELE_FAILED "550 Delete operation failed.\r\n"    
    #define COMMAND_550_SIZE_FAILED "550 SIZE operation failed.\r\n"  
    #define COMMAND_550_SIZE_NOT_GET_FILE "550 Could not get file size.\r\n"
    //响应
    class util{
    public:
        

    };
}
//配置线程池（服务器、任务调度）、缓冲区、端口号
 #endif