#ifndef LINKQUE_H
#define LINKQUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <pthread.h>

/*传感器采集的数据*/
typedef struct sensor_data{
    char temper[32];
    char elect[64];
    char oil[32];
    char A[32];
    char V[32];
    char P[32];
    //char gps[128];
    char time[32];
    char N[32];
    char E[32];
}SENSOR_DATA;

/*队列结点中的数据*/
typedef struct queue_data{
    pthread_t   id_of_sender;
    char        name_of_sender[32];
    pthread_t   id_of_recver;
    char        name_of_recver[32];
    SENSOR_DATA sensordata;
}QUEUE_DATATYPE;

/*队列结点*/
typedef struct queuenode {
    QUEUE_DATATYPE data;
    struct queuenode *next;
}QueueNode;

/*队列*/
typedef struct queue {
    QueueNode *head;
    int clen;
    QueueNode *tail;
}LinkQueue;

LinkQueue *CreateLinkQueue(void);//创建队列
int DestroyLinkQueue(LinkQueue *queue);//销毁队列
int QuitLinkQueue(LinkQueue *queue);//出队
int EnterLinkQueue(LinkQueue *queue, QUEUE_DATATYPE *data);//入队
int IsEmptyLinkQueue(LinkQueue *queue);//判空
QueueNode*  GetHeadLinkQueue(LinkQueue *queue);//获取队头元素
int GetSizeLinkQueue(LinkQueue *queue);

#endif // LINKQUE_H
