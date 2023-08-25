/**
 * @file task_worker.cpp
 * @author songhaikun
*/
#include "ace/OS.h"
#include "ace/Message_Block.h"
#include "task_worker.h"

#include <iostream>
namespace MyFtpServer {
    int Task_Worker::svc(void) {
        while(1) {
            ACE_Message_Block *mb = NULL;
            if(-1 == this->getq(mb)) {
                continue;
            }
            process_task(mb); // 处理mb
        }
        return 0;
    }
    void Task_Worker::process_task(ACE_Message_Block *mb) {
        //处理数据
        
        // std::cout << "-----------processing task----------" << std::endl;

        // ACE_DEBUG((LM_DEBUG, ACE_TEXT("Processing task: %s length %d/n"),
        //     mb->rd_ptr(), mb->length()));
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("Processing task: %s"), mb->rd_ptr()));
        
        ACE_OS::sleep(3);
        mb->release();
    }
}