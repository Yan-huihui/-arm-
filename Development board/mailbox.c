#include "mailbox.h"
#include <pthread.h>
#include <stdio.h>

/*创建线程邮箱*/
LinkList *createMailbox(void)
{
    LinkList *temp = CreateLinkList();/*创建链表头*/
    return temp;
}

/*销毁线程邮箱*/
int destoryMailbox(LinkList *mbs)
{
    while(mbs->head != NULL)
    {
        DestroyLinkQueue(mbs->head->data.queue);/*销毁队列*/
        mbs->head->data.queue = NULL;           /*置空队列*/
        pthread_mutex_destroy(&mbs->head->data.mutex);/*销毁互斥锁*/
        DeleteHeadLinkList(mbs);                /*销毁链表*/
    }
    DestroyLinkList(&mbs);
    return 0;
}

/*添加线程任务*/
int addPthreadInMailbox(LinkList *mbs, char *name, th_fun fun)
{
    LINK_DATATYPE *temp = malloc(sizeof (LINK_DATATYPE));/*申请数据类型空间*/
    strcpy(temp->name, name);

    temp->fun = fun;                            /*填写参数*/
    pthread_mutex_init(&temp->mutex, NULL);
    temp->queue = CreateLinkQueue();


    int ret_pthread = pthread_create(&(temp->tid),NULL, fun, mbs);/*起线程*/
    if(0 != ret_pthread)
    {
        perror("fail to pthread_create in addPthreadInMailbox");
        return -1;
    }

    printf("pthread_create:%s tid:%ld\n", name, temp->tid);

    InsertHeadLinkList(mbs, temp);/*头插*/
    free(temp);

    return 0;
}

/*按照线程名称在线程邮箱寻找*/
int findbyname(LINK_DATATYPE * data,void * arg)
{
    return 0 == strcmp(data->name, (char *)arg);
}

/*按照tid在线程邮箱寻找*/
int findbytid(LINK_DATATYPE * data,void * arg)
{
    return (data->tid == *(pthread_t *)arg);
}

/*线程邮箱数据发送*/
int sendDataToMailbox(LinkList *mbs, char *recver, SENSOR_DATA *getdata)
{
    QUEUE_DATATYPE data;
    LinkNode * temp_rec = FindLinkList(mbs, findbyname, recver);/*寻找接受方线程在链表中保存数据位置*/
    if(NULL == temp_rec)
    {
        printf("can't find recver_name in sendDataToMailbox\n");
        return -1;
    }
    data.id_of_recver = temp_rec->data.tid;
    strcpy(data.name_of_recver, temp_rec->data.name);

    data.id_of_sender = pthread_self();
    LinkNode * temp_sed = FindLinkList(mbs, findbytid, &data.id_of_sender);/*寻找自己线程在链表中保存数据位置*/
    if(NULL == temp_sed)
    {
        printf("can't find tid in sendDataToMailbox\n");
        return -1;
    }
    strcpy(data.name_of_sender, temp_sed->data.name);

    memcpy(&data.sensordata, getdata, sizeof (SENSOR_DATA));

    printf("recver %s %ld\n", data.name_of_recver, data.id_of_recver);
    printf("sender %s %ld\n", data.name_of_sender, data.id_of_sender);
    printf("sendDataInMailbox :\n time:%s N:%s E:%s oil:%s elect:%s temper:%s\n", data.sensordata.time
           ,data.sensordata.N, data.sensordata.E, data.sensordata.oil, data.sensordata.elect, data.sensordata.temper);

    pthread_mutex_lock(&temp_rec->data.mutex);
    EnterLinkQueue(temp_rec->data.queue, &data);    /*入队*/
    pthread_mutex_unlock(&temp_rec->data.mutex);

    return 0;
}

int  getDatafromMailbox(LinkNode * pthread_listnode, QueueNode *pthread_get_queuenode)
{
    pthread_mutex_lock(&pthread_listnode->data.mutex);
    memcpy(pthread_get_queuenode, pthread_listnode->data.queue->head, sizeof (QueueNode));/*获取队头元素*/
    QuitLinkQueue(pthread_listnode->data.queue);    /*出队*/
    pthread_mutex_unlock(&pthread_listnode->data.mutex);

    return 0;
}

