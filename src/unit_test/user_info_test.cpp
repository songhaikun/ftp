#include "user_info.h"
#include "constants.h"

#include "ace/SOCK_Stream.h"

#include <gtest/gtest.h>

#include <memory>

using namespace MyFtpServer;

//
class User_Info_Test: public ::testing::Test
{
public:
    std::unique_ptr<User_Info> user_info_handler_;
    
    User_Info_Test() {
        user_info_handler_ = std::unique_ptr<User_Info>(new User_Info());
    }

protected:
    void SetUp() override
    {
        // #undef USE_TASK
    }

    void TearDown()
    {
        user_info_handler_->clear_users_map();
    }
};

TEST_F(User_Info_Test, check_user_test) { 
    std::string username = "hksong";
    std::string password = "123456";
    int res = user_info_handler_->init_users_map();
    EXPECT_EQ(res, USERS_MAP_INIT_SUCCESS);
    res = user_info_handler_->check_user(username, password);
    EXPECT_TRUE(res == HAS_THIS_USER);
}

TEST_F(User_Info_Test, check_user_test_no_such_username) {
    std::string username = "hksong1";
    std::string password = "123456";
    int res = user_info_handler_->init_users_map();
    EXPECT_EQ(res, USERS_MAP_INIT_SUCCESS);
    res = user_info_handler_->check_user(username, password);
    EXPECT_TRUE(res == NO_SUCH_USERNAME);
}

TEST_F(User_Info_Test, check_user_test_password_wrong) {
    std::string username = "hksong";
    std::string password = "1234567";
    int res = user_info_handler_->init_users_map();
    EXPECT_EQ(res, USERS_MAP_INIT_SUCCESS);
    res = user_info_handler_->check_user(username, password);
    EXPECT_TRUE(res == USER_PASSWORD_WRONG);
}