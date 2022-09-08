#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "data_processing.h"
#include <termios.h>

int get_oil(int fd, SENSOR_DATA *data)
{
    char buff[16] = {0};
    tcflush(fd, TCIFLUSH);
    usleep(400000);
    read(fd, buff, 16);
    //usleep(400000);
    //usleep(1041 * 16);
    int byteh, bytel, distance;
    char *temp = buff;
    int i;
    for(i = 0; i < 16; ++i)
    {
        printf("0x%x ",buff[i]);
    }
    printf("\n");
    for(i = 0; i < 16; ++i)
    {
        if(*temp == 0x5a && *(temp + 1) == 0x5a)
        {
            byteh = temp[4];
            bytel = temp[5];
            distance = (byteh << 8) | bytel;
            sprintf(data->oil,"%d mm",distance);
            break;
        }
        else
        {
            temp++;
        }
    }
    return 0;
}

int get_temper(int fd, SENSOR_DATA *data)
{
    int adc = 0;
    read(fd, &adc, 1);
    double temper = 0.0;
    temper = 0.1903 * adc - 30.7565;
    sprintf(data->temper,"%.1f",temper);

    return 0;
}

int get_gps(int fd, SENSOR_DATA *data)
{
    int ret = 0;
    char buff[1024] = {0};
    ret = read(fd, buff, sizeof (buff));
    printf("gps ret = %d\n",ret);
//    printf("%s\n",buff);
    sleep(2);
    char *flag = strstr(buff,"$GNGGA");
    int hour;
    if(flag != NULL)
    {
        printf("flag = %s\n",flag);
        char *time = index(flag,',');
        time+=1;
        char *end = index(time,',');
        *end = '\0';
        if(end - time < 2)
        {
            time = "search";
            sprintf(data->time,"%s",time);
        }
        else
        {
            printf("get time %s",time);
            hour = (time[0] - '0') *10 + (time[1] - '0') + 8;
            strtok(time, ".");
            sprintf(data->time,"%02d%s",hour, time+2);
            printf("process time :%s", data->time);
        }

        printf("time = %s\n",time);

        char *latitude = end+1;
        end = index(latitude,',');
        *end = '\0';
        printf("latitude = %s\n",latitude);

        char *latitude_dir = end+1;
        end = index(latitude_dir,',');
        *end = '\0';
        printf("latitude_dir = %s\n",latitude_dir);

        char *longtitude = end+1;
        end = index(longtitude,',');
        *end = '\0';
        printf("longtitude = %s\n",longtitude);

        char *longtitude_dir = end+1;
        end = index(longtitude_dir,',');
        *end = '\0';
        printf("longtitude_dir = %s\n",longtitude_dir);


        sprintf(data->N, "%s",latitude);
        sprintf(data->E, "%s",longtitude);
        //sprintf(data->gps,"time:%d%s %s:%s %s:%s",time+2, latitude_dir, latitude, longtitude_dir,longtitude);
    }
    else
    {
        return -1;
    }
    return 0;
}
