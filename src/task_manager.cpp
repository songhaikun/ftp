/**
 * @file task_manager.cpp
 * @author songhaikun
*/
#include "ace/Message_Block.h"
#include "task_manager.h"
#include "task_worker.h"
#include "constants.h"
#include <iostream>
namespace MyFtpServer {
    //管理器
    int Task_Manager::svc(void) {

        Task_Worker task_tp;

        task_tp.activate(THR_NEW_LWP | THR_JOINABLE, TASK_THREAD_POOL_SIZE); //轻量级 + joinable + 线程池大小
        
        while(1) {
            // 从队列中不断取消息块加入线程组
            ACE_Message_Block *mb = NULL;
            if(0 > this->getq(mb)) {
                task_tp.msg_queue()->deactivate();
                task_tp.wait();
            }
            task_tp.putq(mb);
        }
        return 0;

    }
}