#include "linkque.h"

LinkQueue *CreateLinkQueue(void)
{
    LinkQueue *temp = malloc(sizeof (LinkQueue));
    if(NULL == temp)
    {
        perror("fail to malloc in linkqueue");
        return NULL;
    }
    temp->head = NULL;
    temp->tail = NULL;
    temp->clen = 0;
    return temp;
}

int EnterLinkQueue(LinkQueue *queue, QUEUE_DATATYPE *data)
{
    QueueNode *newnode = malloc(sizeof (QueueNode));
    if(NULL == newnode)
    {
        perror("fail to malloc in EnterLinkQueue");
        return -1;
    }
    memcpy(&newnode->data, data, sizeof (QUEUE_DATATYPE));
    newnode->next = NULL;

    if(IsEmptyLinkQueue(queue))
    {
        queue->head = newnode;
    }
    else
    {
        queue->tail->next = newnode;
    }
    queue->tail = newnode;
    queue->clen++;
    return 0;
}

int IsEmptyLinkQueue(LinkQueue *queue)
{
    return 0 == queue->clen;
}

int QuitLinkQueue(LinkQueue *queue)
{
    if(IsEmptyLinkQueue(queue))
    {
        printf("fail to QuitLinkQueue IsEmptyLinkQueue\n");
        return -1;
    }
    QueueNode *temp = queue->head;
    queue->head = queue->head->next;
    if(NULL == queue->head)
    {
        queue->tail = NULL;
    }
    free(temp);
    queue->clen--;
    return 0;
}

QueueNode *GetHeadLinkQueue(LinkQueue *queue)
{
    return queue->head;
}

int DestroyLinkQueue(LinkQueue *queue)
{
    while(queue->head)
    {
        QuitLinkQueue(queue);
    }
    free(queue);
    return 0;
}
int GetSizeLinkQueue(LinkQueue *queue)
{
    return queue->clen;
}
