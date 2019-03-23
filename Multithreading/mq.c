#include "mq.h"
#include <stdio.h>


mqd_t mqueue_init(const char* queue_name, int queue_size, int message_size)
{
    mqd_t msg_q_des;
    struct mq_attr queue_attr;
    //printf("queue name in %s is %s\n",__func__,queue_name);
    //printf("queue size in %s is %d\n",__func__,queue_size);
    //printf("queue name in %s is %s",__func__,queue_name);
    queue_attr.mq_maxmsg  = queue_size;
    queue_attr.mq_msgsize = message_size;
    //queue_attr.mq_flags   = O_NONBLOCK;
   
    msg_q_des = mq_open(queue_name, O_CREAT | O_RDWR, 0666, &queue_attr);

    return msg_q_des;
}

