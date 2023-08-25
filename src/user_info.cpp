/**
 * @file user_info.cpp
 * @author songhaikun
*/
#include "user_info.h"
#include "constants.h"

#include "ace/Log_Msg.h"

#include <fstream>
namespace MyFtpServer {
    std::unordered_map<std::string, std::string> User_Info::users_map_ = std::unordered_map<std::string, std::string>(); //init
    std::string User_Info::users_config_path = USERS_CONFIG_PATH;

    User_Info::User_Info(): is_logging_(false), current_dir_(BASE_DIR){}

    void User_Info::login() {
        current_dir_ = "/home/hksong/" + username_ + "/ftp_home/";
        // current_dir_ = "/home/hksong/hksong/ftp_home/";
        this->is_logging_ = true;
        
    }
    void User_Info::logout() {
        username_ = "";
        current_dir_ = BASE_DIR;
        this->is_logging_ = false;
    }
    std::string User_Info::get_current_dir() {
        return current_dir_;
    }

    int User_Info::set_current_dir(std::string dir) {
        current_dir_ = dir;
        return 1;
    }

    int User_Info::set_username(std::string username) {
        if("" != username) {
            username_ = username;
            return 1;
        }
        return 0;
    }

    std::string User_Info::get_username() {
        return username_;
    }

    int User_Info::init_users_map() {
        std::ifstream iftm1(users_config_path);
        if(!iftm1.is_open()) {
            ACE_DEBUG ( (LM_ERROR, "open error\n"));
            return USERS_CONFIG_OPEN_ERROR;
        }
        std::string username, password;
        while(iftm1 >> username >> password) {  //init users_map_
            users_map_[username] = password;
        }
        iftm1.close();
        return USERS_MAP_INIT_SUCCESS;
    }

    void User_Info::clear_users_map() {
        users_map_.clear();
    }

    int User_Info::check_username(std::string username) { //static
        if("" != username) {
            auto iter = users_map_.find(username);
            if(users_map_.end() != iter) {
                return HAS_THIS_USERNAME;
            }
        }
        return NO_SUCH_USERNAME;
    }

    int User_Info::check_user(std::string username, std::string password) {  //static
        if("" != username) {
            auto iter = users_map_.find(username);
            if(users_map_.end() != iter) {
                // return HAS_THIS_USERNAME;
                if(iter->second == password) {
                    return HAS_THIS_USER;
                }
                else {
                    return USER_PASSWORD_WRONG;
                }
            }
        }
        return NO_SUCH_USERNAME;
    }
    
}