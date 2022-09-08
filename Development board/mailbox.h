#ifndef MAILBOX_H
#define MAILBOX_H

#include "linklist.h"
#include "linkque.h"

/*创建线程邮箱*/
LinkList *createMailbox(void);

/*销毁线程邮箱*/
int destoryMailbox(LinkList *mbs);

/*添加线程任务*/
int addPthreadInMailbox(LinkList *mbs, char *name, th_fun fun);

/*线程邮箱数据发送*/
int sendDataToMailbox(LinkList *mbs, char *recver, SENSOR_DATA* getdata);

/*线程中从线程邮箱接收数据*/
int  getDatafromMailbox(LinkNode * pthread_listnode, QueueNode *pthread_get_queuenode);

/*按照tid在线程邮箱寻找*/
int findbytid(LINK_DATATYPE * data,void * arg);

/*按照线程名称在线程邮箱寻找*/
int findbyname(LINK_DATATYPE * data,void * arg);
#endif // MAILBOX_H
