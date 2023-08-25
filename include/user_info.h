/**
 * @file user_info.h
 * @author songhaikun
*/
#ifndef __USER_INFO_H_
#define __USER_INFO_H_
#include <string>
#include <unordered_map>

namespace MyFtpServer{
    /**
     * @brief 检测用户是否登陆宏函数
    */
    #define CHECK_LOGGING() \
    if(!user_info_->is_logging()) {\
        send_response(COMMAND_500_LOGIN);\
        return;\
    }\
    else{}\

    /**
     * @brief 用户信息管理类，包括验证登陆、保存用户当前路径与连接信息等
    */
    class User_Info {
    public:
        /// @brief 构造函数，初始化用户登陆状态、家目录
        User_Info();

        /// @brief 析构函数
        ~User_Info() {};

        /// @brief 判断是否登陆
        bool is_logging() {return is_logging_;}
        
        /// @brief 改变登陆状态为已登录并设置家目录
        void login();
        
        /// @brief 改变登陆状态为未登陆并清空家目录、
        void logout();

        /**
         * @brief 得到用户当前目录位置
         * @return std::string 用户当前所处目录
        */
        std::string get_current_dir();

        /**
         * @brief 设置当前目录
         * @param dir 需要设置的目录
         * @return 设置是否成功
        */
        int set_current_dir(std::string dir);

        /**
         * @brief 设置用户名
         * @param username 用户名
         * @return int 1 成功，0 失败
        */
        int set_username(std::string username);
        
        /**
         * @brief 获取用户名
         * @return string 用户名字符串
        */
        std::string get_username();

        /**
         * @brief 初始化用户账户密码信息映射
         * @return int USERS_CONFIG_OPEN_ERROR错误，打开用户信息文件失败， USERS_MAP_INIT_SUCCESS 成功。
        */
        static int init_users_map();

        /// @brief 清空用户账户密码信息映射
        static void clear_users_map();

        /**
         * @brief 检查用户名是否已注册
         * @param username 用户名
         * @return int HAS_THIS_USERNAME 已注册， NO_SUCH_USERNAME 未注册
        */
        static int check_username(std::string username);

        /**
         * @brief 检查用户账号是否注册，密码是否匹配
         * @param username 用户名
         * @param password 密码
         * @return int NO_SUCH_USERNAME 用户名未注册， USER_PASSWORD_WRONG 密码错误， HAS_THIS_USER 验证成功
        */
        static int check_user(std::string username, std::string password);

    private:
        /// @brief 是否登陆
        bool is_logging_;
        /// @brief 用户名
        std::string username_;
        /// @brief 当前路径
        std::string current_dir_;
        /// @brief 重命名前保存原文件名用于错误恢复等
        std::string old_file_name_;
        /// @brief 所有用户账户名、密码映射
        static std::unordered_map<std::string, std::string> users_map_;
        /// @brief 用户信息文件路径
        static std::string users_config_path;
    };
}
#endif