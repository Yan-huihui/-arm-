#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "drive_application.h"


int IC_TTY_addToLibIc()
{
	printf("IC_TTY_addToLibIc");

	return 0;
}


int IC_TTY_Option_Set(int fd,int baud_rate,int data_bits,char parity,int stop_bits)
{

       struct termios new_cfg, old_cfg;
	   
       int speed;
	   
       /*保存并测试现有串口参数设置，在这里如果串口号等出错，会有相关出错信息*/
       if(tcgetattr(fd, &old_cfg) != 0)       /*该函数得到fd指向的终端配置参数，并将它们保存到old_cfg变量中，成功返回0，否则-1*/
	{
	
              perror("tcgetttr");
			  
              return -1;
			  
       }
 
        /*设置字符大小*/
       new_cfg = old_cfg;   
		
       cfmakeraw(&new_cfg); /*配置为原始模式*/ 
	   
       new_cfg.c_cflag &= ~CSIZE; /*用位掩码清空数据位的设置*/  
 
      /*设置波特率*/
       switch(baud_rate)
	   	
       {
		case 2400:
			speed = B2400;			
			break;
 
		case 4800:			
			speed = B4800;			
			break;
 
		case 9600:
			speed = B9600;			
			break;
 
		case 19200:			
			speed = B19200;			
			break;
 
		case 38400:			
			speed = B38400;			
			break;
 
		default:			
		case 115200:			
			speed = B115200;			
			break;
		
       }
 
       cfsetispeed(&new_cfg, speed); //设置输入波特率
	   
       cfsetospeed(&new_cfg, speed); //设置输出波特率
 
       /*设置数据长度*/
       switch(data_bits)
	   	
       {
		case 5:
			new_cfg.c_cflag &= ~CSIZE;//屏蔽其它标志位
			new_cfg.c_cflag |= CS5;
			break;
		
		case 6:
			new_cfg.c_cflag &= ~CSIZE;//屏蔽其它标志位
			new_cfg.c_cflag |= CS6;
	   		break;
		
		case 7:
			new_cfg.c_cflag &= ~CSIZE;//屏蔽其它标志位
			new_cfg.c_cflag |= CS7;
			break;
 
		default:			
		case 8:
			new_cfg.c_cflag &= ~CSIZE;//屏蔽其它标志位
			new_cfg.c_cflag |= CS8;
			break;
		
       }
 
	/*设置奇偶校验位*/
	switch(parity)
	{
		default:
			
		case 'n':
			
		case 'N': //无校验
			
		{
			new_cfg.c_cflag &= ~PARENB;
			
			new_cfg.c_iflag &= ~INPCK;
		}	
		break;
 
		case 'o': //奇校验
			
		case 'O':
			
		{
			new_cfg.c_cflag |= (PARODD | PARENB);
			
			new_cfg.c_iflag |= INPCK;
		}		
		break;
 
		case 'e': //偶校验
 
		case 'E':
 
		{
 
			new_cfg.c_cflag |=  PARENB;
 
			new_cfg.c_cflag &= ~PARODD;
 
			new_cfg.c_iflag |= INPCK;
 
		}
		break;
		
	}
 
       /*设置停止位*/
       switch(stop_bits)
 
       {
 
		default:
		case 1:
			new_cfg.c_cflag &= ~CSTOPB;
			break;
 
		case 2:
			new_cfg.c_cflag |= CSTOPB;
			break;
		
       }
 
 
 
      /*设置等待时间和最小接收字符*/
       new_cfg.c_cc[VTIME] = 0; /* 读取一个字符等待1*(1/10)s */
 
       new_cfg.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */
 
 
	/*处理未接收字符*/
	tcflush(fd, TCIFLUSH); //溢出数据可以接收，但不读
 
 
 
       /* 激活配置 (将修改后的termios数据设置到串口中)
         * TCSANOW：所有改变立即生效
         */
       if((tcsetattr(fd, TCSANOW, &new_cfg))!= 0)
 
       {
       
              perror("tcsetattr");
 
              return -1;
			  
       }
 
	return 0;
}


int IC_TTY_Init(const char* pTtyName,int nSpeed,int nBits,char nEvent,int nStop)
{
	if (NULL == pTtyName)
	{
		printf("pTtyName is NULL");
		return -1;
	}

	int nComFd = 0;

	nComFd = open(pTtyName, O_RDWR|O_NOCTTY);
	if (nComFd <= 0 )
	{
		printf("Couldn't open %s", pTtyName);

		return -2;
	}
	else
	{
		printf("open %s success!", pTtyName);	
	}


	IC_TTY_Option_Set(nComFd,nSpeed,nBits,nEvent,nStop);

	//	fcntl( nComFd, F_SETFL, FNDELAY );

	return nComFd;
}

int IC_TTY_Destroy(int nComFd)
{
	if (nComFd > 0)
	{
		close(nComFd);
	}

	return 0;
}


void* send_th(void* arg)
{
	int fd =  *(int*)arg;
	unsigned char buf[256]="aaaaaabbbbb";
	printf("thread id %lu,fd is %d\n",pthread_self(),fd);	
//	int i = 0 ;

	printf("data ready...\n");
	
	int ret;
	while(1)
	{

		ret = write(fd,buf,strlen(buf));
		if(ret>0)
		{
			printf("send %d byte\n",ret);
		}
		else
		{
        	tcflush(fd, TCOFLUSH); //TCOFLUSH刷新写入的数据但不传送  
			printf("send 0 byte or error\n");
			
		}


		sleep(3);
	}
}
void* recv_th(void* arg)
{
	int fd = *(int *)arg;
	printf("thread id %lu,fd is %d\n",pthread_self(),fd);	
	char buf[256]={0};
	int ret;
	fd_set rd_set,temp_set;
	FD_ZERO(&rd_set);
	FD_ZERO(&temp_set);
	FD_SET(fd,&temp_set);
	struct timeval tm;
	while(1)
	{
		tm.tv_sec = 5;
		tm.tv_usec = 0 ;
		rd_set = temp_set;
		ret  = select(1024,&rd_set,NULL,NULL,&tm);
		if(ret >0)
		{
			ret = read(fd,buf,256);
			if(ret>0)
			{
				int i  = 0 ;
				for(i = 0 ;i<ret;i++)
				{
					printf("0x%x ",buf[i]);
					fflush(stdout);
				}
				printf("\n");
			}
		}
		else if( 0 ==ret)
		{

			printf("time out ,no data recv...\n");
		}
		else
		{
			printf("select error\n");
			return 0;
		}
	}

}
//int main(int argc,char**argv)
//{
//	int fd =  IC_TTY_Init("/dev/ttySAC1",9600,8,'N',1);
//	printf("thread id %lu,fd is %d\n",pthread_self(),fd);
//	pthread_mutex_init(&mutex, NULL);

//	char buff[1024] = {0};
//	int i;
//	for(i = 0; i < 10; ++i)
//	{
//		bzero(buff, sizeof(buff));

//		pthread_mutex_lock(&mutex);
//		read(fd, buff, sizeof(buff));
//		pthread_mutex_unlock(&mutex);

//		printf("%s\n\n",buff);
//		usleep(500000);
//		sleep(1);
//	}
	
//	pthread_mutex_destroy(&mutex);
//	close(fd);
//	pthread_t tid1,tid2;
//	pthread_create(&tid1,NULL,send_th,&fd);
//	pthread_create(&tid2,NULL,recv_th,&fd);
//	pthread_join(tid1,NULL);
//	pthread_join(tid2,NULL);
//	return 0;
//}
