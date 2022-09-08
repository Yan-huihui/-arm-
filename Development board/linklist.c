#include "linklist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


LinkList *CreateLinkList(void)
{
    LinkList *temp = malloc(sizeof (LinkList));
    if(NULL == temp)
    {
        perror("fail to malloc linklist");
        return NULL;
    }

    temp->head = NULL;
    temp->clen = 0;
    return temp;
}

int InsertHeadLinkList(LinkList *list, LINK_DATATYPE *data)
{
    LinkNode * newnode = malloc((sizeof (LinkNode)));
    if(NULL == newnode)
    {
        perror("fail to malloc linknode");
        return -1;
    }

    memcpy(&newnode->data, data, sizeof (LINK_DATATYPE));

    newnode->next = NULL;
    newnode->prev = NULL;

    if(!IsEmptyLinkList(list))
    {
        newnode->next = list->head;
        list->head->prev = newnode;
    }
    list->head = newnode;

    list->clen++;

    return 0;
}

int IsEmptyLinkList(LinkList *list)
{
    return 0 == list->clen;
}

int ShowLinkList(LinkList *list, SHOW_DIRE dire)
{
    //printf("过来打印了\n");
    LinkNode *temp = list->head;
    int size = GetSizeLinkList(list);
    int i;
    if(!IsEmptyLinkList(list))
    {
        if(SHOW_FORWARD == dire)
        {
            for(i = 0; i < size; ++i)
            {
                printf("%s \n",temp->data.name);
                temp = temp->next;
            }
        }
        else
        {
            while (NULL != temp->next)
            {
                temp = temp->next;
            }

            for(i = 0; i < size; ++i)
            {
                 printf("%s \n",temp->data.name);
                temp = temp->prev;
            }
        }
    }
    return 0;
}

int GetSizeLinkList(LinkList *list)
{
    return list->clen;
}

LinkNode *FindLinkList(LinkList *list, int (*cmp_fun) (LINK_DATATYPE *, void *), void *arg)
{
    LinkNode* temp = list->head;
    int size = GetSizeLinkList(list);
    int i ;
//    printf("\nfindlinklist \n");
    for(i = 0 ;i<size;i++)
    {
//        printf("FindLinkList: data->tid:%ld arg->tid:%ld\n\n",temp->data.tid, *(pthread_t *)arg);
        //if(0 == strcmp(temp->data.name,name))
        if(cmp_fun(&temp->data,arg))
        {
//            printf("\nend findlinklist\n");
            return temp;
        }
        temp= temp->next;
    }
    return NULL;
}


int DeleteLinkList(LinkList *list, int (*cmp_fun) (LINK_DATATYPE *, void *), void *arg)
{
    LinkNode* temp = FindLinkList(list, cmp_fun, arg);
    if(NULL == temp)
    {
        return -1;
    }
    else
    {
        if(temp->next)
        {
            temp->next->prev = temp->prev;
        }
        if(temp->prev)
        {
            temp->prev->next = temp->next;
        }
        else
        {
            list->head = temp->next;
        }
    }
    free(temp);
    list->clen--;
    return 0;
}

int ModifyLinkList(LinkList *list, int (*cmp_fun)(LINK_DATATYPE *, void *), void *arg, LINK_DATATYPE *data)
{
    LinkNode* temp = FindLinkList(list, cmp_fun, arg);
    if(NULL == temp)
    {
        return -1;
    }
    else
    {
        memcpy(temp, data, sizeof (LINK_DATATYPE));
    }
    return 0;
}

int DestroyLinkList(LinkList **list)
{
    LinkNode* temp = (*list)->head;
    while (temp)
    {
        (*list)->head = (*list)->head->next;
        free(temp);
        temp = (*list)->head;
    }

    *list = NULL;
    free(*list);
    return 0;
}

int InsertTailLinkList(LinkList *list, LINK_DATATYPE *data)
{
    LinkNode * newnode = malloc((sizeof (LinkNode)));
    if(NULL == newnode)
    {
        perror("fail to malloc linknode");
        return -1;
    }

    memcpy(&newnode->data, data, sizeof (LINK_DATATYPE));

    newnode->next = NULL;
    newnode->prev = NULL;

    if(!IsEmptyLinkList(list))
    {
        LinkNode* temp = list->head;
        while (temp->next)
        {
            temp = temp->next;
        }
        temp->next = newnode;
        newnode->prev = temp;
    }
    else
    {
        list->head = newnode;
    }

    list->clen++;
    return 0;
}

int DeleteHeadLinkList(LinkList *list)
{
    printf("%d\n", list->clen);
    if(list->head != NULL)
    {
        LinkNode* temp = list->head;
        list->head = list->head->next;
        //list->head->prev = NULL;
        list->clen--;
        free(temp);
        temp = NULL;
    }
    return 0;
}
