/**
 * @file task_worker.h
 * @author songhaikun
*/
#ifndef __TASK_WORKER_H_
#define __TASK_WORKER_H_

#include "ace/Task.h"
#include "ace/Synch.h"
#include "ace/Message_Block.h"


//handle_data 负责处理数据， Task_Worker负责解析ftp并返回相应报文

namespace MyFtpServer{
    /**
     * @brief 任务具体逻辑处理类
    */
    class Task_Worker:public ACE_Task<ACE_MT_SYNCH>{
    public:
        virtual int svc(void);
    private:
        void process_task(ACE_Message_Block *mb); //处理函数
    };
}

#endif