#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "linklist.h"
#include "linkque.h"
#include "mailbox.h"
#include "drive_application.h"
#include "data_processing.h"
#include "framebuff.h"

static char EQUID[8] = "u10000";

void* dataCollection(void* arg)
{
    sleep(1);
    pthread_t ret_tid = pthread_self();
    printf("pthread:%s tid:%ld\n", "dataCollection", ret_tid);
    LinkNode * pthread_listnode = FindLinkList((LinkList *)arg, findbytid, &ret_tid);
    if(NULL == pthread_listnode)
    {
        printf("can't find tid in dataCollection\n");
        return NULL;
    }

    int fd_oil =  IC_TTY_Init("/dev/ttyUSB0",9600,8,'N',1);
    int fd_temper = open("/dev/temper", O_RDWR);
    int fd_gps = IC_TTY_Init("/dev/ttySAC1",9600,8,'N',1);
//    int fd_elect = IC_TTY_Init("/dev/ttySCA2",9600,8,'N',1);

    SENSOR_DATA data;
    int i = 0;
    while(1)
    {
        bzero(&data, sizeof(SENSOR_DATA));

        get_oil(fd_oil,&data);
        get_gps(fd_gps, &data);
       get_temper(fd_temper, &data);

//        sprintf(data.gps,);
//        sprintf(data.time,"160201");
//        sprintf(data.N, "12346");
//        sprintf(data.E, "456789");
//        sprintf(data.oil, "12mm");
        sprintf(data.V, "%d", i);
        sprintf(data.A, "%d", i-1);
        sprintf(data.P, "%d", i+1);
//        sprintf(data.temper, "25");

        printf("\ndataCollection\n");
        printf("%s\n", data.time);
        printf("%s\n", data.N);
        printf("%s\n", data.E);
        printf("%s\n", data.oil);
        printf("%s\n", data.V);
        printf("%s\n", data.A);
        printf("%s\n", data.P);
        printf("%s\n", data.temper);

        sendDataToMailbox(arg, "data_save", &data);
        sendDataToMailbox(arg, "data_socket_send", &data);
        sendDataToMailbox(arg, "image_display", &data);
        sleep(1);
        i++;
    }
    return NULL;
}

void* dataSave(void* arg)
{
    sleep(1);
    pthread_t ret_tid = pthread_self();
    printf("pthread:%s tid:%ld\n", "dataSave", ret_tid);
    LinkNode * pthread_listnode = FindLinkList((LinkList *)arg, findbytid, &ret_tid);
    if(NULL == pthread_listnode)
    {
        printf("can't find tid in dataSave\n");
        return NULL;
    }

    FILE * fd = fopen("./data.txt","w");
    QueueNode pthread_get_queuenode;
    while(1)
    {
        if(!IsEmptyLinkQueue(pthread_listnode->data.queue))
        {
            printf("queue len = %d\n",pthread_listnode->data.queue->clen);
            printf("dataSave1\n");
            bzero(&pthread_get_queuenode, sizeof (QueueNode));

            getDatafromMailbox(pthread_listnode, &pthread_get_queuenode);

            printf("%s\n", "in dataSave");
            printf("in dataSave %s\n", pthread_get_queuenode.data.sensordata.time);
            printf("in dataSave %s\n", pthread_get_queuenode.data.sensordata.N);
            printf("in dataSave %s\n", pthread_get_queuenode.data.sensordata.E);
            printf("in dataSave %s\n", pthread_get_queuenode.data.sensordata.oil);
            printf("in dataSave %s\n", pthread_get_queuenode.data.sensordata.temper);
            printf("in dataSave %s\n", pthread_get_queuenode.data.sensordata.V);
            printf("in dataSave %s\n", pthread_get_queuenode.data.sensordata.A);
            printf("in dataSave %s\n", pthread_get_queuenode.data.sensordata.P);

            if(0 > strcmp(pthread_get_queuenode.data.sensordata.time, "000002"))
            {
                fclose(fd);
                fd = fopen("./data.txt","w");
            }

            fprintf(fd, "time:%s N:%s E:%s oil:%s temper:%s V:%s A:%s P:%s\n", pthread_get_queuenode.data.sensordata.time
                    ,pthread_get_queuenode.data.sensordata.N,pthread_get_queuenode.data.sensordata.E
                    ,pthread_get_queuenode.data.sensordata.oil,pthread_get_queuenode.data.sensordata.temper
                    ,pthread_get_queuenode.data.sensordata.V,pthread_get_queuenode.data.sensordata.A,
                    pthread_get_queuenode.data.sensordata.P);
            fflush(fd);
        }
    }

    return NULL;
}

void* dataSocketSend(void* arg)
{
    sleep(1);
    pthread_t ret_tid = pthread_self();
    printf("pthread:%s tid:%ld\n", "dataSocketSend", ret_tid);
    LinkNode * pthread_listnode = FindLinkList((LinkList *)arg, findbytid, &ret_tid);
    if(NULL == pthread_listnode)
    {
        printf("can't find tid in dataSocketSend\n");
        return NULL;
    }


    int conn = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == conn)
    {
        perror("fail to socket");
        return NULL;
    }

    struct sockaddr_in ser, cli;
    bzero(&ser, sizeof(ser));
    bzero(&cli, sizeof(cli));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(50000);
    ser.sin_addr.s_addr = inet_addr("192.168.1.200");
    int ret = connect(conn, (struct sockaddr *)&ser, sizeof(ser));
    if(-1 == ret)
    {
        perror("fail to connect");
        return NULL;
    }

    QueueNode pthread_get_queuenode;
    while(1)
    {
        if(!IsEmptyLinkQueue(pthread_listnode->data.queue))
        {
            printf("queue len = %d\n",pthread_listnode->data.queue->clen);
            bzero(&pthread_get_queuenode, sizeof (QueueNode));

            getDatafromMailbox(pthread_listnode, &pthread_get_queuenode);

            printf("in dataSocketSend %s\n", "in dataSocketSend");
            printf("in dataSocketSend %s\n", pthread_get_queuenode.data.sensordata.time);
            printf("in dataSocketSend %s\n", pthread_get_queuenode.data.sensordata.N);
            printf("in dataSocketSend %s\n", pthread_get_queuenode.data.sensordata.E);
            printf("in dataSocketSend %s\n", pthread_get_queuenode.data.sensordata.oil);
            printf("in dataSocketSend %s\n", pthread_get_queuenode.data.sensordata.temper);
            printf("in dataSocketSend %s\n", pthread_get_queuenode.data.sensordata.V);
            printf("in dataSocketSend %s\n", pthread_get_queuenode.data.sensordata.A);
            printf("in dataSocketSend %s\n", pthread_get_queuenode.data.sensordata.P);

            char buff[256] = {0};
            sprintf(buff, "equid:%s time:%s N:%s E:%s oil:%s temper:%s V:%s A:%s P:%s\n",EQUID, pthread_get_queuenode.data.sensordata.time
                    ,pthread_get_queuenode.data.sensordata.N,pthread_get_queuenode.data.sensordata.E
                    ,pthread_get_queuenode.data.sensordata.oil,pthread_get_queuenode.data.sensordata.temper
                    ,pthread_get_queuenode.data.sensordata.V,pthread_get_queuenode.data.sensordata.A,
                    pthread_get_queuenode.data.sensordata.P);

            int sd_ret = send(conn, buff, strlen(buff), 0);
            if(sd_ret < 0)
            {
                printf("fail to send\n");
                //return NULL;
            }

        }
    }
    return NULL;
}

void* imageDisplay(void* arg)
{
    sleep(1);
    pthread_t ret_tid = pthread_self();
    printf("pthread:%s tid:%ld\n", "imageDisplay", ret_tid);
    LinkNode * pthread_listnode = FindLinkList((LinkList *)arg, findbytid, &ret_tid);
    if(NULL == pthread_listnode)
    {
        printf("can't find tid in imageDisplay\n");
        return NULL;
    }

    init_fb("/dev/fb0");
    UTF8_INFO utf8_info;
    bzero(&utf8_info, sizeof(UTF8_INFO));
    strcpy(utf8_info.path, ZIKUK_FILE_BIG);
    utf8_info.width = 32;
    utf8_info.height = 32;
    init_utf8(&utf8_info);

    QueueNode pthread_get_queuenode;
    while(1)
    {
        if(!IsEmptyLinkQueue(pthread_listnode->data.queue))
        {

            printf("queue len = %d\n",pthread_listnode->data.queue->clen);
            bzero(&pthread_get_queuenode, sizeof (QueueNode));

            getDatafromMailbox(pthread_listnode, &pthread_get_queuenode);

            printf("%s\n", "in imageDisplay");
            printf("in imageDisplay %s\n", pthread_get_queuenode.data.sensordata.time);
            printf("in imageDisplay %s\n", pthread_get_queuenode.data.sensordata.N);
            printf("in imageDisplay %s\n", pthread_get_queuenode.data.sensordata.E);
            printf("in imageDisplay %s\n", pthread_get_queuenode.data.sensordata.oil);
            printf("in imageDisplay %s\n", pthread_get_queuenode.data.sensordata.temper);
            printf("in imageDisplay %s\n", pthread_get_queuenode.data.sensordata.V);
            printf("in imageDisplay %s\n", pthread_get_queuenode.data.sensordata.A);
            printf("in imageDisplay %s\n", pthread_get_queuenode.data.sensordata.P);

            draw_bmp(0, 0, 240, 320, "./a.bmp");
            //strtok(pthread_get_queuenode.data.sensordata.time, ".");
            show_utf8_str(&utf8_info, 40, 30, pthread_get_queuenode.data.sensordata.time, 0x00000000, 0x00ffffff);
            strtok(pthread_get_queuenode.data.sensordata.N, ".");
            show_utf8_str(&utf8_info, 40, 60, pthread_get_queuenode.data.sensordata.N, 0x00000000, 0x00ffffff);
            strtok(pthread_get_queuenode.data.sensordata.E, ".");
            show_utf8_str(&utf8_info, 40, 90, pthread_get_queuenode.data.sensordata.E, 0x00000000, 0x00ffffff);
            show_utf8_str(&utf8_info, 40, 120, pthread_get_queuenode.data.sensordata.oil, 0x00000000, 0x00ffffff);
            show_utf8_str(&utf8_info, 40, 150, pthread_get_queuenode.data.sensordata.temper, 0x00000000, 0x00ffffff);
            show_utf8_str(&utf8_info, 40, 180, pthread_get_queuenode.data.sensordata.V, 0x00000000, 0x00ffffff);
            show_utf8_str(&utf8_info, 40, 210, pthread_get_queuenode.data.sensordata.A, 0x00000000, 0x00ffffff);
            show_utf8_str(&utf8_info, 40, 240, pthread_get_queuenode.data.sensordata.P, 0x00000000, 0x00ffffff);
        }
    }
    return NULL;
}

int main()
{
    LinkList * mailbox = createMailbox();

    addPthreadInMailbox(mailbox, "data_collection", dataCollection);
    addPthreadInMailbox(mailbox, "data_save", dataSave);
    addPthreadInMailbox(mailbox, "data_socket_send", dataSocketSend);
    addPthreadInMailbox(mailbox, "image_display", imageDisplay);

    while(1)
    {

    }
    destoryMailbox(mailbox);

    printf("Hello World!\n");
    return 0;
}
