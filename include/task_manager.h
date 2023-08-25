/**
 * @file task_manager.h
 * @author songhaikun
*/
#ifndef __TASK_MANAGER_H_
#define __TASK_MANAGER_H_


#include "ace/Task.h"
#include "ace/Synch.h"
namespace MyFtpServer{
    /**
     * @brief 任务管理类
    */
    class Task_Manager:public ACE_Task<ACE_MT_SYNCH> {
    public:
        virtual int svc(void);
    };
}

//task server
#endif