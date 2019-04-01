/************************************************************************************************
* File name   : mq.c                                                                            *
* Authors     : Nachiket Kelkar and Puneet Bansal                                               *
* Description : The function definition used for mqueue.                                        *
* Tools used  : GNU make, gcc, arm-linux-gcc                                                    *
************************************************************************************************/
#include "mq.h"
#include <stdio.h>


mqd_t mqueue_init(const char* queue_name, int queue_size, int message_size)
{
    mqd_t msg_q_des;
    struct mq_attr queue_attr;

    queue_attr.mq_maxmsg  = queue_size;
    queue_attr.mq_msgsize = message_size;
    queue_attr.mq_flags   = O_NONBLOCK;
    msg_q_des = mq_open(queue_name, O_CREAT | O_RDWR | O_NONBLOCK, 0666, &queue_attr);

    return msg_q_des;
}

