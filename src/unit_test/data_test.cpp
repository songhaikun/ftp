#include "data_handler.h"
#include <gtest/gtest.h>
#include <memory>

using namespace MyFtpServer;

class Data_Test : public ::testing::Test
{
public:
    std::unique_ptr<Data_Handler> data_handler_;
    
    Data_Test() {
        data_handler_ = std::unique_ptr<Data_Handler>(new Data_Handler("127.0.0.1", 12121, 3));
    }

protected:

};

TEST_F(Data_Test, file_link_init_test)
{
    data_handler_->set_file_path("/home/hksong/haikunsong/ftp_home/testfile.txt");
    int res = data_handler_->file_link_init(1);
    EXPECT_EQ(res, 0);
    res = data_handler_->file_link_init(0);
    EXPECT_EQ(res, 0);
}

TEST_F(Data_Test, send_file_test)
{
    data_handler_->set_file_path("/home/hksong/haikunsong/ftp_home/test1.txt");
    int res = data_handler_->send_file();
    EXPECT_EQ(res, -1);
}

TEST_F(Data_Test, data_link_init_test)
{
    int res = data_handler_->data_link_init();
    EXPECT_EQ(res, -1);
}

// TEST_F(Data_Test, recv_file_test)
// {
//     data_handler_->set_file_path("/home/hksong/haikunsong/ftp_home/testsize.txt");

// }