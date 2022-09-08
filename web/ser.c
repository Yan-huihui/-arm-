#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <signal.h>

int main(int argc, const char *argv[])
{
	//打开要写入的文件
//	FILE *fp = fopen("./getfile.png", "w+");
	
	int listfd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == listfd)
	{
		perror("fail to socket");
		return -1;
	}

	struct sockaddr_in ser, cli;
	bzero(&ser, sizeof(ser));
	bzero(&cli, sizeof(cli));
	ser.sin_family = AF_INET;
	ser.sin_port = htons(50000);
	ser.sin_addr.s_addr = inet_addr("192.168.1.3");
	int ret = bind(listfd, (struct sockaddr *)&ser, sizeof(ser));
	if(-1 == ret)
	{
		perror("fail to bind");
		return -1;
	}

	//三次握手排队数
	listen(listfd, 3);
	//通信套接字
	socklen_t len = sizeof(cli);
	int conn = accept(listfd, (struct sockaddr *)&cli, &len);

	//接收cli到ser数据
	while(1)
	{
		char buff[1024] = {0};
		int rd_ret = recv(conn, buff, sizeof(buff), 0);
		if(rd_ret <= 0)
		{
			continue;
//			break;
		}
		printf("%s\n", buff);
//		fwrite(buff, sizeof(char), rd_ret, fp);
//		fflush(fp);	
	}
	close(listfd);
	close(conn);
	return 0;
}
