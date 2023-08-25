#include "command_handler.h"
#include "constants.h"
#include <memory>
#include <gtest/gtest.h>
using namespace MyFtpServer;

#define CHECK_CMD(cmd_t, cmds, res)\
    cmd_handler_->handle_##cmd_t(cmds);\
    std::string res = cmd_handler_->get_and_clear_response_list();\

class Command_Test: public ::testing::Test
{
public:
    std::unique_ptr<Command_Handler> cmd_handler_;
    std::string resp_;
    Command_Test(){
        cmd_handler_ = std::unique_ptr<Command_Handler>(new Command_Handler(0));
    }
    
protected:
    void login_user() {
        char buf[32] = "user haikunsong";
        cmd_info_ cmds = {buf, 4, 5, 15};
        cmd_handler_->handle_user(cmds);
        char buf_ps[32] = "pass dingjia";
        cmd_info_ cmds_ps = {buf_ps, 4, 5, 12};
        cmd_handler_->handle_pass(cmds_ps);
        cmd_handler_->get_and_clear_response_list();
    }

    void logout_user() {
        char buf[32] = "quit";
        cmd_info_ cmds = {buf, 4, -1, 4};
        cmd_handler_->handle_quit(cmds);
        cmd_handler_->get_and_clear_response_list();
    }
    void SetUp() override
    {
        //login
        login_user();
    }

    void TearDown()
    {
        //logout
        logout_user();
    }
};

TEST_F(Command_Test, parseline_test_cmd_para)
{
    char buf[1024] = "command para\r\n";
    cmd_info_ cmds = {buf, 0, 0, 0};
    int res = cmd_handler_->parse_line(cmds);
    std::string cmd = std::string(cmds.buf, cmds.cmd_end);
    std::string para = std::string(cmds.buf + cmds.para_start);
    EXPECT_EQ(res, PARSE_LINE_OK_CMD_AND_PARA);
    EXPECT_TRUE("command" == cmd);
    EXPECT_TRUE("para"    == para);
}

TEST_F(Command_Test, parseline_test_cmd)
{
    char buf[1024] = "command  \r\n";
    cmd_info_ cmds = {buf, 0, 0, 0};
    int res = cmd_handler_->parse_line(cmds);
    std::string cmd = std::string(cmds.buf, cmds.cmd_end);
    EXPECT_EQ(res, PARSE_LINE_OK_ONLY_CMD);
    EXPECT_EQ(cmds.para_start, -1);
    EXPECT_TRUE("command" == cmd);
}

TEST_F(Command_Test, parseline_test_error)
{
    char buf[1024] = "command";
    cmd_info_ cmds = {buf, 0, 0, 0};
    int res = cmd_handler_->parse_line(cmds);
    EXPECT_EQ(res, PARSE_LINE_ERROR);
    EXPECT_EQ(cmds.len, 0);
}

TEST_F(Command_Test, process_cmd_test)
{
    char buf[1024] = "user haikunsong";
    cmd_info_ cmds = {buf, 4, 5, 15};
    auto res = cmd_handler_->process_cmd(cmds);
    EXPECT_TRUE(&Command_Handler::handle_user == res);
}

TEST_F(Command_Test, process_cmd_test_error)
{
    char buf[1024] = "usea haikunsong";
    cmd_info_ cmds = {buf, 4, 5, 15};
    auto res = cmd_handler_->process_cmd(cmds);
    EXPECT_TRUE(nullptr == res);
}

TEST_F(Command_Test, send_and_get_reponse_local_test)
{
    cmd_handler_->send_response_local("hello ");
    int res_i = cmd_handler_->send_response_local("world");
    std::string res_s1 = cmd_handler_->get_and_clear_response_list();
    std::string res_s2 = cmd_handler_->get_and_clear_response_list();
    EXPECT_EQ(res_i, 1);
    EXPECT_TRUE(res_s1 == "hello world" && res_s2 == "");
}

TEST_F(Command_Test, get_real_path_test_user1)
{
    std::string path1 = "..";
    cmd_handler_->get_real_path(path1);
    EXPECT_TRUE("/home/hksong/haikunsong/" == path1);
    std::string path2 = "./testdir";
    cmd_handler_->get_real_path(path2);
    EXPECT_TRUE("/home/hksong/haikunsong/ftp_home/testdir/" == path2);
}

TEST_F(Command_Test, get_real_path_test_user2)
{
    logout_user();
    //log in user hksong
    char buf[32] = "user hksong";
    cmd_info_ cmds = {buf, 4, 5, 11};
    char buf_ps[32] = "pass 123456";
    cmd_info_ cmds_ps = {buf_ps, 4, 5, 11};
    CHECK_CMD(user, cmds, res);
    CHECK_CMD(pass, cmds_ps, res_pass);

    std::string path1 = "..";
    cmd_handler_->get_real_path(path1);
    EXPECT_TRUE("/home/hksong/hksong/" == path1);
    std::string path2 = "./testdir";
    cmd_handler_->get_real_path(path2);
    EXPECT_TRUE("/home/hksong/hksong/ftp_home/testdir/" == path2);
}

TEST_F(Command_Test, list_dir_test)
{
    std::list<std::string> list;
    DIR* dir = nullptr;
    std::string path = "/home/hksong/haikunsong/ftp_home/";
    dir = opendir(path.c_str());
    cmd_handler_->list_dir(list, path.c_str(), dir);
    EXPECT_TRUE(list.size() > 1);
}

TEST_F(Command_Test, handle_user_test)
{
    char buf[1024] = "user haikunsong";
    cmd_info_ cmds = {buf, 4, 5, 15};
    CHECK_CMD(user, cmds, res);
    EXPECT_TRUE(res == COMMAND_331);
}

TEST_F(Command_Test, handle_user_test_error)
{
    char buf[1024] = "user haikun";
    cmd_info_ cmds = {buf, 4, 5, 15};
    CHECK_CMD(user, cmds, res);
    EXPECT_TRUE(res == COMMAND_530_LOGIN_FAILED);
}

TEST_F(Command_Test, handle_pass_test)
{
    char buf[32] = "user hksong";
    cmd_info_ cmds = {buf, 4, 5, 11};
    char buf_ps[32] = "pass 123456";
    cmd_info_ cmds_ps = {buf_ps, 4, 5, 11};
    CHECK_CMD(user, cmds, res);
    CHECK_CMD(pass, cmds_ps, res_pass);
    EXPECT_TRUE(res_pass == COMMAND_230);
}

TEST_F(Command_Test, handle_pass_test_error)
{
    char buf[32] = "user hksong";
    cmd_info_ cmds = {buf, 4, 5, 11};
    char buf_ps[32] = "pass 12345";
    cmd_info_ cmds_ps = {buf_ps, 4, 5, 10};
    CHECK_CMD(user, cmds, res);
    CHECK_CMD(pass, cmds_ps, res_pass);
    EXPECT_TRUE(res_pass == COMMAND_530_LOGIN_FAILED);
}

TEST_F(Command_Test, handle_syst_test)
{
    char buf[32] = "syst";
    cmd_info_ cmds = {buf, 4, -1, 4};
    CHECK_CMD(syst, cmds, res);
    EXPECT_TRUE(res == COMMAND_215);
}

TEST_F(Command_Test, handle_epsv_test)
{
    char buf[32] = "epsv";
    cmd_info_ cmds = {buf, 4, -1, 4};
    CHECK_CMD(epsv, cmds, res);
    EXPECT_TRUE(res == COMMAND_550_NOT_SUPPORT);
}

TEST_F(Command_Test, handle_eprt_test)
{
    char buf[32] = "eprt";
    cmd_info_ cmds = {buf, 4, -1, 4};
    CHECK_CMD(eprt, cmds, res);
    EXPECT_TRUE(res == COMMAND_550_NOT_SUPPORT);
}

TEST_F(Command_Test, handle_feat_test)
{
    char buf[32] = "feat";
    cmd_info_ cmds = {buf, 4, -1, 4};
    CHECK_CMD(feat, cmds, res);
    EXPECT_TRUE(res == COMMAND_211);
}

TEST_F(Command_Test, handle_pasv_test)
{
    char buf[32] = "pasv";
    cmd_info_ cmds = {buf, 4, -1, 4};
    CHECK_CMD(pasv, cmds, res);
    res = res.substr(0, 3);
    EXPECT_TRUE(res == "227");
}

TEST_F(Command_Test, handle_pwd_test)
{
    char buf[32] = "pwd";
    cmd_info_ cmds = {buf, 3, -1, 3};
    CHECK_CMD(pwd, cmds, res);
    EXPECT_TRUE(res == std::string(COMMAND_257_PWD_BEIGN) + "/home/hksong/haikunsong/ftp_home/" + COMMAND_257_PWD_BEIGN_END);
}

TEST_F(Command_Test, handle_cwd_test)
{
    char buf[32] = "cwd ..";
    cmd_info_ cmds = {buf, 3, 4, 6};
    char buf_pwd[32] = "pwd";
    cmd_info_ cmds_pwd = {buf_pwd, 3, -1, 3};

    CHECK_CMD(cwd, cmds, res);
    EXPECT_TRUE(res == COMMAND_250_CWD_SUCCESS);
    CHECK_CMD(pwd, cmds_pwd, res_pwd);
    EXPECT_TRUE(res_pwd == std::string(COMMAND_257_PWD_BEIGN) + "/home/hksong/haikunsong/" + COMMAND_257_PWD_BEIGN_END);
}

TEST_F(Command_Test, handle_mkd_and_rmd_test)
{
    char buf[32] = "rmd testdir1";
    cmd_info_ cmds = {buf, 3, 4, 12};
    char buf_mkd[32] = "mkd testdir1";
    cmd_info_ cmds_mkd = {buf_mkd, 3, 4, 12};

    CHECK_CMD(rmd, cmds, res);
    CHECK_CMD(mkd, cmds_mkd, res_mkd);
    EXPECT_TRUE(res == COMMAND_250_DELE_SUCCESS && res_mkd == COMMAND_257_MKD_SUCCESS);
}

TEST_F(Command_Test, handle_size_test)
{
    char buf[32] = "size testsize.txt";
    cmd_info_ cmds = {buf, 4, 5, 17};
    CHECK_CMD(size, cmds, res);
    EXPECT_TRUE(res == "213 6\r\n");
}

TEST_F(Command_Test, handle_type_test)
{
    char buf[32] = "type a";
    cmd_info_ cmds = {buf, 4, 5, 6};
    CHECK_CMD(type, cmds, res);
    EXPECT_TRUE(res == COMMAND_SUCCESS);
}

TEST_F(Command_Test, handle_retr_test_error)
{
    char buf[32] = "retr";
    cmd_info_ cmds = {buf, 4, -1, 4};
    CHECK_CMD(retr, cmds, res);
    EXPECT_TRUE(res == COMMAND_500_FAILED);
}

TEST_F(Command_Test, handle_stor_test_error)
{
    char buf[32] = "stor";
    cmd_info_ cmds = {buf, 4, -1, 4};
    CHECK_CMD(stor, cmds, res);
    EXPECT_TRUE(res == COMMAND_500_FAILED);
}

TEST_F(Command_Test, handle_port_test)
{
    char buf[32] = "port 127,0,0,1,100,100";
    cmd_info_ cmds = {buf, 4, 5, 22};
    CHECK_CMD(port, cmds, res);
    EXPECT_TRUE(res == COMMAND_SUCCESS);
}