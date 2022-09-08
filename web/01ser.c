#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <time.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include <sys/epoll.h>
#include "urldeconde.h"
#include <signal.h>

int line = 0;

typedef struct sockaddr* (SA);
//epoll添加fd
int add_fd(int epfd, int fd)
{
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = fd;

	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
	if(-1 == ret)
	{
		perror("add_fd");
		return ret;
	}
}
//发送响应头
int send_head(int conn,char* filename)
{

    struct stat st;
    int ret = stat(filename,&st);
    if(-1 ==ret)
    {
        printf("path error %s\n",filename);
        return 1;
    }

    char temp[100]={0};
    char * send_cmd[6]={NULL};
    send_cmd[0]= "HTTP/1.1 200 OK\r\n";
    send_cmd[1]="Server: myserver\r\n";
    send_cmd[2]="Date: Sat, 23 Jul 2022 07:52:15 GMT\r\n";
    send_cmd[3]="Content-Type: text/html; charset=utf-8\r\n";
    send_cmd[4]=temp;
    sprintf(send_cmd[4],"Content-Length: %lu\r\n",st.st_size);

    send_cmd[5]="Connection: closed\r\n\r\n";

    int i = 0 ;
    for(i = 0 ;i<6;i++)
    {
        send(conn,send_cmd[i],strlen(send_cmd[i]),0);
    }

    return 0;

}
//发送文件
int send_file(int conn,char* filename)
{
    send_head(conn,filename);
    int srcfd = open(filename,O_RDONLY);
    if(-1 == srcfd)
    {
        perror("open error");
        return 1;
    }
    while(1)
    {

        char buf[1024]={0};
        int rd_ret = read(srcfd,buf,sizeof(buf));
        if(rd_ret<=0)
        {
            break;
        }
        send(conn,buf,rd_ret,0);
    }
    close(srcfd);
    return 0;

}

//用户登陆查询回调
int find_user(void *arg, int col, char **result, char ** title)
{
	*(int *)arg = 1;
	return 0;
}

//用户登陆查询
int user_login(int conn,char *url)
{
	sqlite3 *db = NULL;
	int ret = sqlite3_open("./user.db", &db);
	if(SQLITE_OK != ret)
	{
		fprintf(stderr,"sqlite3_open:%s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 1;
	}
	
	char sql_cmd[1024] = {0};
	char *name =NULL;
	char *pass=NULL;
	char *end =NULL;
	char *errmsg = NULL;
	name = index(url,'=');
	name+=1;

	end = index(name,'&');
	*end='\0';

	pass = index(end+1,'=');
	pass+=1;
	
	sprintf(sql_cmd, "select * from user where username = \"%s\" and password = \"%s\";",name,pass);
	int find_flag = 0;
	ret = sqlite3_exec(db, sql_cmd, find_user, &find_flag, &errmsg);
	if(SQLITE_OK != ret)
	{
		fprintf(stderr,"sqlite3_exec:%s\n", sqlite3_errmsg(db));
		sqlite3_free(errmsg);
		sqlite3_close(db);
		return 1;
	}

	if(find_flag == 1)
	{
		send_file(conn,"./search.html");
	}
	else
	{
		send_file(conn,"./error_user.html");
	}
	sqlite3_free(errmsg);
	sqlite3_close(db);
	return 0;
}

//写网页头
int wire_html_head(char *file_path)
{
	if(NULL == file_path)
	{
		file_path = "./search_results.html";
	}
	FILE *fd = fopen(file_path, "w");
	if(NULL == fd)
	{
		perror("open html fill");
	}
	fprintf(fd, "<!DOCTYPE html>\r\n<html>\r\n\t<head>\r\n\t\t<meta charset=\"utf-8\">\r\n\t\t<title>搜索</title>\r\n\t</head>\r\n\t<body>\r\n\t<h1 align='center'>\r\n\t搜索设备\r\n\t</h1>\r\n\t\t<form action='search'><center>\r\n\t\t<input type='text' name='equid' required='required' placeholder='输入你需要查询的设备'>\r\n\t\t<input type='submit'></center>\r\n\t\t</form>\r\n\t\t<center><table cellpadding=\"20\">\r\n");
	fclose(fd);
}
//写网页结尾

int wire_html_end(char *file_path)
{
	if(NULL == file_path)
	{
		file_path = "./search_results.html";
	}

	FILE *fd = fopen(file_path, "a");
	if(NULL == fd)
	{
		perror("open html fill");
	}

	fseek(fd, 0, SEEK_END);
	fprintf(fd, "\n\t\t</table></center>\r\n\t</body>\r\n</html>\r\n");
	fclose(fd);
}

//搜索设备信息回调函数
int find_goods(void *arg, int col, char **result, char **title)
{
	*(int *)arg = 1;
	
	FILE *fd = fopen("./search_results.html", "a");
	if(NULL == fd)
	{
		perror("open html fill");
	}

	fseek(fd, 0, SEEK_END);
	char buff[1024] = {0};

	if(line == 0)
	{
		fprintf(fd,"\t\t<br><tr><td>序号</td><td>时间</td><td>经度</td><td>纬度</td><td>油位</td><td>温度</td><td>电压</td><td>电流</td><td>功率</td></tr>\r\n");

	}
	fprintf(fd,"\t\t<br><tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>\r\n",
			result[0],result[1], result[2], result[3], result[4], result[5], result[6], result[7], result[8]);
	
	line++;
	fclose(fd);

	return 0;
}

//写寻找结果页
int search_equ(char *url)
{
	//search?goods=%E4%B8%89%E6%98%9F
	sqlite3 *db = NULL;
	int ret = sqlite3_open("./data.db", &db);
	if(SQLITE_OK != ret)
	{
		fprintf(stderr,"sqlite3_open:%s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 1;
	}
	
	char sql_cmd[1024] = {0};
	char *equid = NULL;
	char *errmsg = NULL;
	equid = index(url,'=');
	equid += 1;

	sprintf(sql_cmd, "select * from %s;",equid);
	int find_flag = 0;
	line = 0;
	ret = sqlite3_exec(db, sql_cmd, find_goods, &find_flag, &errmsg);
	if(SQLITE_OK != ret)
	{
		fprintf(stderr,"sqlite3_exec:%s\n", sqlite3_errmsg(db));
		sqlite3_free(errmsg);
		sqlite3_close(db);
		FILE *fd = fopen("./search_results.html", "a");
		if(NULL == fd)
		{
			perror("open html fill");
		}

		printf("not find goods\n");	
		fprintf(fd, "<br>\t\t<center>抱歉没有找到该设备<br> 换个设备id试试</center>");
		fclose(fd);
		return 1;
	}

	return 0;
}

//处理接受到数据，执行不同操作
int process_html_data(int conn, char *buf)
{
	char * method=NULL;
	char* url = NULL;
	char* ver=NULL;
	method = strtok(buf," ");
	url=strtok(NULL," ");
	ver=strtok(NULL,"\r");

	printf("URL: %s\n",url);
	if(0 == strcmp(url,"/"))
	{
		send_file(conn,"./login.html");
	}
	if(NULL != index(url, '.'))
	{
		send_file(conn ,url+1);
	}
	if(0 == strncmp(url,"/login",6))
	{
		user_login(conn, url);
	}
	else if(0 == strncmp(url,"/search?equid",13))
	{
		wire_html_head(NULL);
		search_equ(url);
		wire_html_end(NULL);
		send_file(conn, "./search_results.html");
	}
}

int process_html_accect(void)
{
    int listfd  = socket(AF_INET,SOCK_STREAM,0);
    if(-1 == listfd)
    {    
        perror("socket");
        exit(1);
    }

    struct sockaddr_in ser,cli;
    bzero(&ser,sizeof(ser));
    bzero(&cli,sizeof(cli));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(80);
    ser.sin_addr.s_addr =INADDR_ANY ;

    int on = 1;
    setsockopt(listfd, SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
   //setsockopt(listfd, IPPROTO_TCP,TCP_NODELAY,&on,sizeof(on));
    int ret = bind(listfd,(SA)&ser,sizeof(ser));
    if(-1 == ret)
    {
        perror("bind");
        exit(1);
    }
    listen(listfd,3);

    int len = sizeof(cli);
	int conn;

	int  epfd = epoll_create(10);
	if(-1 == epfd)
	{
		perror("epoll_create");
		return 1;
	}

	struct epoll_event rev[10];
	add_fd(epfd, listfd);
    while(1)
    {
		int i;
		int ep_ret = epoll_wait(epfd, rev, 10, -1);
		for(i = 0; i < ep_ret; i++)
		{
			if(rev[i].data.fd == listfd)
			{
				conn = accept(listfd,(SA)&cli,&len);
				if(-1 == conn)
				{
					perror("accept");
					continue;
				}
				add_fd(epfd, conn);
			}
			else
			{
				char buf[4096] = {0};
				bzero(buf,sizeof(buf));
				int ret= recv(rev[i].data.fd ,buf,sizeof(buf),0);
				if(ret<=0)
				{
					epoll_ctl(epfd, EPOLL_CTL_DEL, rev[i].data.fd, NULL);
					close(rev[i].data.fd);
					continue;
				}
				
				process_html_data(rev[i].data.fd, buf);
				epoll_ctl(epfd, EPOLL_CTL_DEL, rev[i].data.fd, NULL);
				close(rev[i].data.fd);
			}
		}
    }
    close(listfd);
    return 0;
}

int process_save_data(int con, char *buf)
{
	printf("%s\n",buf);
	//	"equid:%d time:%s N:%s E:%s oil:%s temper:%s V:%s A:%s P:%s\n"
	strtok(buf, ":");
	char *equid = strtok(NULL, " ");
	
	strtok(NULL, ":");
	char *time = strtok(NULL, " ");

	strtok(NULL, ":");
	char *N = strtok(NULL, " ");

	strtok(NULL, ":");
	char *E = strtok(NULL, " ");

	strtok(NULL, ":");
	char *oil = strtok(NULL, " ");

	strtok(NULL, ":");
	char *temper = strtok(NULL, " ");

	strtok(NULL, ":");
	char *V = strtok(NULL, " ");

	strtok(NULL, ":");
	char *A = strtok(NULL, " ");

	strtok(NULL, ":");
	char *P = strtok(NULL, "\n");
	
	sqlite3* db = NULL;
    int ret = sqlite3_open("./data.db",&db);
    if(SQLITE_OK != ret)
    {
        fprintf(stderr,"sqlite3_open:%s\n",sqlite3_errmsg(db));
        sqlite3_close(db);
		return -1;
	}
	char* errmsg = NULL;
	char sql_cmd1[1024] = {0};
	char sql_cmd2[1024] = {0};
	sprintf(sql_cmd1,"insert into %s (time, N, E, oil, temper, V, A, P) values (\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\")",equid, time, N, E, oil, temper, V, A, P);
	ret = sqlite3_exec(db, sql_cmd1, NULL, NULL, &errmsg);
	if(SQLITE_OK != ret)
	{
		fprintf(stderr,"sqlite3_exec wirte mean%s:%s\n",sqlite3_errmsg(db),sql_cmd1);
		
		sprintf(sql_cmd2, "create table %s(id INTEGER PRIMARY KEY ASC, time char, N char, E char, oil char, temper char, V char, A char, P char);", equid);
		sqlite3_exec(db, sql_cmd2, NULL, NULL, &errmsg);
		sqlite3_exec(db, sql_cmd1, NULL, NULL, &errmsg); 
	} 
	sqlite3_free(errmsg);
	sqlite3_close(db);

}

int process_save_accect(void)
{
    int listfd  = socket(AF_INET,SOCK_STREAM,0);
    if(-1 == listfd)
    {    
        perror("socket");
        exit(1);
    }

    struct sockaddr_in ser,cli;
    bzero(&ser,sizeof(ser));
    bzero(&cli,sizeof(cli));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(50000);
    ser.sin_addr.s_addr =INADDR_ANY ;

    int on = 1;
    setsockopt(listfd, SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
   //setsockopt(listfd, IPPROTO_TCP,TCP_NODELAY,&on,sizeof(on));
    int ret = bind(listfd,(SA)&ser,sizeof(ser));
    if(-1 == ret)
    {
        perror("bind");
        exit(1);
    }
    listen(listfd,3);

    int len = sizeof(cli);
	int conn;

	int  epfd = epoll_create(10);
	if(-1 == epfd)
	{
		perror("epoll_create");
		return 1;
	}

	struct epoll_event rev[10];
	add_fd(epfd, listfd);
    while(1)
    {
		int i;
		int ep_ret = epoll_wait(epfd, rev, 10, -1);
		for(i = 0; i < ep_ret; i++)
		{
			if(rev[i].data.fd == listfd)
			{
				conn = accept(listfd,(SA)&cli,&len);
				if(-1 == conn)
				{
					perror("accept");
					continue;
				}
				add_fd(epfd, conn);
			}
			else
			{
				char buf[4096] = {0};
				bzero(buf,sizeof(buf));
				int ret= recv(rev[i].data.fd ,buf,sizeof(buf),0);
				if(ret<=0)
				{
					epoll_ctl(epfd, EPOLL_CTL_DEL, rev[i].data.fd, NULL);
					close(rev[i].data.fd);
					continue;
				}
				
				process_save_data(rev[i].data.fd, buf);
				//epoll_ctl(epfd, EPOLL_CTL_DEL, rev[i].data.fd, NULL);
				//close(rev[i].data.fd);
			}
		}
    }
    close(listfd);
    return 0;
}

int main(int argc, const char *argv[])
{
	int i, n = 2;
	pid_t pid[2];
	for(i = 0; i < n; i++)
	{
		pid[i] = fork();
		if(pid[i] > 0)
		{
		
		}
		else if(0 == pid[i])
		{
			switch(i)
			{
				case 0: process_save_accect(); break;
				case 1: process_html_accect(); break;
			}
		}
	}
	char cmd[32];
	scanf("%s",cmd);
	kill(pid[0], 9);
	kill(pid[1], 9);
	//process_save_accect();	
	//process_html_accect();
	return 0;
}
