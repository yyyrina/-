#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <time.h>
#include "proto.h"

#define row_begin  3
#define row_end   21
#define col_begin 5
#define col_end   30

struct user_message              //用户结构体
{
    char Username[20];
    char Password[20];
    int  cmd;
    int flag;
    char target_user[20];
    char content[50];
    int sockfd;
}user;

int socketfd;
static void Login(int sockfd);
static void chooseface(int sockfd);

void End()
{
    printf("\n\t\t\033[33;32m please any key to continue... \033[0m\n\n");
    printf("\t\t");
    while(getchar()!='\n');
    getchar();
}

void setpos(int x,int y)
{
	printf("\033[%d;%dH",y+1,x+1);
}

void setscreen(void)
{
	int i,j;

	printf("\033[2J");
	setpos(0,0);
	for(i = row_begin ; i <= row_end ; i++)
	{
		for(j = col_begin ; j <= col_end ; j++)
		{
			if(i == row_begin || i == row_end )
			{
				setpos(2*j,i);
				printf("\033[30;45m==");
			}
			else if(j == col_begin || j == col_end)
			{
				setpos(2*j,i);		
				printf("\033[30;45m||");
			}
		}
	}
	printf("\033[0m\n\n");
}

static int Reg(int sockfd)
{
	printf("\033[0m");
	setscreen();
	setpos(col_begin+28,row_begin+1);
	printf("\033[30;47m注册");
	setpos(col_begin+10,row_begin+5);
	printf("请输入用户名:");
	setpos(col_begin+10,row_begin+8);
	printf("请输入密码:");
	setpos(col_begin+24,row_begin+5);
	scanf("%s",user.Username);
	setpos(col_begin+22,row_begin+8);
	scanf("%s",user.Password);

     user.flag = 0;
     write(sockfd,&user,sizeof(user));
	 setpos(col_begin+10,row_begin+11);
	 printf("注册成功!");
    while(getchar()!='\n');
    getchar();

}

static void Mainface(int sockfd)
{
	printf("\033[0m");
	setscreen();
	setpos(col_begin+23,row_begin+1);
	printf("\033[30;47m欢迎登入聊天室");
	setpos(col_begin+10,row_begin+4);
	printf("1.登陆");
	setpos(col_begin+10,row_begin+7);
	printf("2.注册");
	setpos(col_begin+10,row_begin+10);
	printf("0.退出");
	setpos(col_begin+10,row_begin+13);
	printf("请输入:");
	setpos(col_begin+18,row_begin+13);
	scanf("%d",&user.cmd);

	switch(user.cmd)
    {
        case 1:
           Login(sockfd);
           break;
	case 2:
           Reg(sockfd);
           break;
        case 0:
           exit(1);
        default:
           break;
    }

}

void Addfriend(int sockfd)     //添加好友
{
    char buf[20];
	printf("\033[0m");
	setscreen();
	setpos(col_begin+10,row_begin+4);
	printf("请输入要添加的好友名称:");
	setpos(col_begin+34,row_begin+4);
    scanf("%s",user.target_user);
    write(sockfd,&user,sizeof(user));
    read(sockfd,buf,sizeof(buf));
    if(strcmp(buf,"SUCCESS")==0)
    {
		setpos(col_begin+10,row_begin+6);
		printf("添加成功!");
    }
    else if(strcmp(buf,"FAIL")==0)
    {
		setpos(col_begin+10,row_begin+6);
		printf("添加失败!该用户不存在");
    }
    End();
}

static void Look(int sockfd)               //查看好友
{
     int len,i = 0;
     struct user_message user1;
	 printf("\033[0m");
     setscreen();
     write(sockfd,&user,sizeof(user));
	 setpos(col_begin+13,row_begin+3);
	 printf("用户名");
	 setpos(col_begin+27,row_begin+3);
	 printf("状态");
     while(len = read(sockfd,&user1,sizeof(user1)) > 3)
     {
        if(user1.cmd == 100)
		{
			break;
		}
        if(user1.flag == 0)
        {
			fflush(NULL);
			setpos(col_begin+13,row_begin+4+i);
			printf("%s",user1.Username);
			setpos(col_begin+27,row_begin+4+i);
			printf("离线");
			i++;
        }
        else if(user1.flag == 1)
        {
			fflush(NULL);
			setpos(col_begin+13,row_begin+4+i);
			printf("%s",user1.Username);
			setpos(col_begin+27,row_begin+4+i);
			printf("在线");
			i++;
        }
     }
     End();
}

// 查看聊天记录
void check(int sockfd)
{
     int len;
     char buf[100];
	 printf("\033[0m");
	 printf("\033[2J");
	 setpos(col_begin+10,row_begin+3);
     printf("\033[33;32m请输入对方用户名:\033[0m");
	 setpos(col_begin+28,row_begin+3);
     scanf("%s",user.target_user);
     write(sockfd,&user,sizeof(user));
    
	 fflush(NULL);
     while(len = read(sockfd,buf,sizeof(buf))>2)
     {
         write(1,buf,sizeof(buf));
     }
     End();
	 chooseface(sockfd);
}
static void checkall(int sockfd)
{
	char buf[100];

	printf("\033[0m");
    printf("\033[2J");
	write(sockfd,&user,sizeof(user));

	fflush(NULL);
	while(read(sockfd,buf,sizeof(buf)) > 2)
	{
		write(1,buf,sizeof(buf));
	}
	End();
	chooseface(sockfd);
}

static void *pth_read()
{
	char pth_buf[100];

	while(1)
	{
		if(read(socketfd,pth_buf,sizeof(pth_buf)) < 0)
		{
			fprintf(stderr,"talk read is:%s\n",strerror(errno));
		}
		printf("\n%s\n",pth_buf);
	}
	pthread_exit(NULL);
}

static void talk(int sockfd)
{
	pthread_t tid;
	char buf[50];
	int err;

	printf("\033[0m");
	printf("\033[2J");
	err = pthread_create(&tid,NULL,pth_read,NULL);
	if(err)
	{
		fprintf(stderr,"pthread_create():%s\n",strerror(err));
		exit(1);
	}
	setpos(col_begin,row_begin);
	printf("\033[30;45m====================欢迎来到聊天室==================\n");
	printf("\033[0m\t\t\t (输入quit退出)\n");
	while(1)
	{
		scanf("%s",buf);
		if(strcmp(buf,"quit") == 0)
		{
			break;
		}
		strcpy(user.content,buf);
		write(socketfd,&user,sizeof(user));
	}
}

static void chooseface(int sockfd)
{
	int i;
	char buf[50];
	
	while(1)
	{
		printf("\033[0m");
		setscreen();
		
		setpos(col_begin+23,row_begin+1);
		printf("\033[30;45m欢迎登入聊天室!");

		setpos(col_begin+12,row_begin+4);
		printf("\033[30;47m1.多人聊天");

		setpos(col_begin+32,row_begin+4);
		printf("\033[30;47m2.私人聊天");

		setpos(col_begin+12,row_begin+7);
		printf("\033[30;47m3.添加好友");

		setpos(col_begin+32,row_begin+7);
		printf("\033[30;47m4.查看好友");

		setpos(col_begin+12,row_begin+10);
		printf("\033[30;47m5.查看群聊记录");

		setpos(col_begin+32,row_begin+10);
		printf("\033[30;47m6.查看私人聊天记录");

		setpos(col_begin+12,row_begin+13);
		printf("\033[30;47m0.退出");
	
		setpos(col_begin+12,row_begin+15);
		scanf("%d",&i);

		switch(i)
		{
			case 1:
				user.cmd = i+5;
				talk(sockfd);
				break;
			case 2:
				user.cmd = i+3;
				setpos(col_begin+20,row_begin+15);
				printf("请输入对方用户名:");
				setpos(col_begin+38,row_begin+15);
				scanf("%s",user.target_user);
				talk(sockfd);
				break;
			case 3:
				user.cmd = i;
				Addfriend(sockfd);
				break;
			case 4:
				user.cmd = i;
				Look(sockfd);
				break;
			case 5:
				user.cmd = 8;
				checkall(sockfd);
				break;
			case 6:
				user.cmd = 7;
				check(sockfd);
				break;
			case 0:
				exit(1);
			default:
				break;
		}
	}
}

static void Login(int sockfd)
{
	char buf[20];
	printf("\033[0m");
	setscreen();

	setpos(col_begin+10,row_begin+5);
	printf("\033[30;47m请输入用户名:");

	setpos(col_begin+10,row_begin+9);
	printf("\033[30;47m请输入密码:");

	setpos(col_begin+24,row_begin+5);
	scanf("%s",user.Username);

	setpos(col_begin+22,row_begin+9);
	scanf("%s",user.Password);

	write(sockfd,&user,sizeof(user));
	read(sockfd,buf,sizeof(buf));
	if(strcmp(buf,"ok") == 0)
	{
		usleep(1000*500);
		chooseface(sockfd);
	}
	else if(strcmp(buf,"fail") == 0)
	{
		setpos(col_begin+19,row_begin+13);
		printf("\033[30;47m登陆失败!");
		End();
	}
}


int main()
{
	struct sockaddr_in caddr;
	
	socketfd = socket(AF_INET,SOCK_STREAM,0);
	if(socketfd < 0)
	{
		perror("socket:()");
		exit(1);
	}

	memset(&caddr,0,sizeof(caddr));
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(atoi(SERVERPORT));
	inet_pton(AF_INET,"IP",&caddr.sin_addr);

	if(connect(socketfd,(void *)&caddr,sizeof(caddr)) < 0)
	{
		perror("connect()");
		exit(1);
	}
	while(1)
	{
		Mainface(socketfd);
	}

	exit(0);
}



