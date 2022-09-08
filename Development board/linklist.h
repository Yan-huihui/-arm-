#ifndef LINKLIST_H
#define LINKLIST_H

#include <pthread.h>
#include "linkque.h"

typedef void*(*th_fun)(void* arg);

typedef struct pthread {
    char name[32];          //线程名称
    pthread_t tid;          //线程id
    th_fun fun;             //线程处理函数
    pthread_mutex_t mutex;  //线程互斥锁
    LinkQueue *queue;       //队列
}LINK_DATATYPE;

typedef struct linknode {
    LINK_DATATYPE data;     //链表数据 存放线程相关
    struct linknode *next,*prev;
}LinkNode;

typedef struct list {
    LinkNode *head;
    int clen;
}LinkList;

typedef enum{
    SHOW_FORWARD,
    SHOW_BACK
}SHOW_DIRE;

LinkList *CreateLinkList(void);//创建链表
int InsertHeadLinkList(LinkList *list, LINK_DATATYPE *data);//头插
int ShowLinkList(LinkList *list, SHOW_DIRE dire);
LinkNode *FindLinkList(LinkList *list, int (*cmp_fun) (LINK_DATATYPE *, void *), void *arg);//查找
int DeleteLinkList(LinkList *list, int (*cmp_fun) (LINK_DATATYPE *, void *), void *arg);//删除
int DeleteHeadLinkList(LinkList *list);//头删
int ModifyLinkList(LinkList *list, int (*cmp_fun) (LINK_DATATYPE *, void *), void *arg, LINK_DATATYPE *data);//修改
int DestroyLinkList(LinkList **list);//销毁表
int InsertTailLinkList(LinkList *list, LINK_DATATYPE *data);//尾插
int IsEmptyLinkList(LinkList *list);
int GetSizeLinkList(LinkList *list);
#endif // LINKLIST_H
