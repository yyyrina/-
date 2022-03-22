#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "proto.h"

enum
{
	LOGIN = 1,
	REG,
	ADD,
	LOOK,
	SIGLE,
	ALL,
	CHOWN,
	CHALL
};

struct user_message
{
	char Username[20];
	char Password[20];
	int cmd;
	int flag;
	char target_user[20];
	char content[50];
	int sockfd;
}user;


struct node
{
	struct user_message data;
	struct node *next;
};

fd_set master;
fd_set rset;
static int fdmax;
static int sd;

static struct node *list_init()
{
	struct node *head = malloc(sizeof(*head));
	if(head == NULL)
	{
		perror("failed!");
		return NULL;
	}
	head->next = head;
	return head;
}

static void Addnode(struct node *head,struct user_message user)
{
	struct node *newnode = malloc(sizeof(*newnode));
	newnode->data = user;
	newnode->data.flag = 0;
	newnode->data.sockfd = 0;
	newnode->next = NULL;

	struct node *p = head;
	while(p->next != head)
		p = p->next;
	p->next = newnode;
	newnode->next = head;
}

static void ReadInfo(struct node *head,struct user_message user)
{
	FILE * fp;
	fp = fopen("user.txt","a+");
	fseek(fp,0,SEEK_SET);
	fscanf(fp,"%s %s",user.Username,user.Password);
	while(!feof(fp))
	{
		Addnode(head,user);
		fscanf(fp,"%s %s",user.Username,user.Password);
	}
}

static void Register(int sockfd,struct user_message user)
{
	printf("%s %s %d\n",user.Username,user.Password,user.cmd);
	FILE * fp;
	fp = fopen("user.txt","a+");
	if(fp == NULL)
	{
		perror("fopen():");
		exit(1);
	}
	fprintf(fp,"%s %s\n",user.Username,user.Password);
	fclose(fp);
	fp = NULL;
}


static void Login(struct node *head,int sockfd,struct user_message user)
{
	ReadInfo(head,user);
	printf("%s %s %d\n",user.Username,user.Password,user.sockfd);
	struct node *p = NULL;
	p = head->next;

	while(p != head)
	{
		if(strcmp(user.Username,p->data.Username) == 0 && strcmp(user.Password,p->data.Password) == 0)
		{
			printf("%d ",sockfd);
			p->data.flag = 1;
			p->data.sockfd = sockfd;
			write(sockfd,"ok",sizeof("ok"));
			return;
		}
		p = p->next;
	}
	write(sockfd,"fail",sizeof("fail"));
}

static void Addfriend(struct node * head,int sockfd,struct user_message user)
{
	char filename[30];
	FILE *fp;
	struct node *p = head->next;

	strcpy(filename,user.Username);
	strcat(filename,".txt");

	while(p != head)
	{
		if(strcmp(user.target_user,p->data.Username) == 0)
		{
			fp = fopen(filename,"a+");
			printf("%s\n",p->data.Username);
			fprintf(fp,"%s %d\n",p->data.Username,p->data.flag);
			write(sockfd,"SUCCESS",sizeof("SUCCESS"));
			fclose(fp);
			fp = NULL;
			return;
		}
		p = p->next;
	}
	write(sockfd,"FAIL",sizeof("FAIL"));
}

static void Look(struct node *head,int sockfd,struct user_message user)
{
	char filename[30];
	FILE * fp;
	
	strcpy(filename,user.Username);
	strcat(filename,".txt");
	printf("打开%s的好友列表",filename);

	fp = fopen(filename,"a+");
	fseek(fp,0L,SEEK_SET);
	fscanf(fp,"%s %d",user.Username,&user.flag);
	printf("%s %d",user.Username,user.flag);
	do
	{
		struct node *p = head->next;
		while(p != head)
		{
			if(strcmp(user.Username,p->data.Username) == 0)
			{
				if(p->data.flag == 1)
				{
					user.flag = 1;
					break;
				}
				if(p->data.flag == 0)
				{
					user.flag = 0;
					break;
				}
			}
			p = p->next;
		}
			printf("%s %d",user.Username,user.flag);
			fflush(NULL);
			write(sockfd,&user,sizeof(user));
			printf("%s %d",user.Username,user.flag);
			fflush(NULL);
			fscanf(fp,"%s %d",user.Username,&user.flag);
		}while(!feof(fp));

		fflush(NULL);
		user.cmd = 100;
		write(sockfd,&user,sizeof(user));
		fclose(fp);
		fp = NULL;
}

static void PrivateTalk(struct node*head,int sockfd)
{
	int j;
	FILE *fp,*fe;
	char str[50],buf[100];
	time_t t;
	struct tm*p;
	struct node *q = head->next;
	char filename1[30],filename2[30];

	t = time(NULL);
	p = localtime(&t);

	strcpy(str,asctime(p));
	strcpy(buf,"[private-talk]");
	strcat(buf," ");
	strcat(buf,str);
	strcat(buf," ");
	strcat(buf,user.Username);
	strcat(buf,"say:");
	strcat(buf,user.content);

	strcat(filename1,user.Username);
	strcat(filename1,user.target_user);
	strcat(filename1,".txt");

	strcat(filename2,user.target_user);
	strcat(filename2,user.Username);
	strcat(filename2,".txt");

	fp = fopen(filename1,"a+");
	fprintf(fp,"%s\n",buf);
	fclose(fp);

	fe = fopen(filename2,"a+");
	fprintf(fe,"%s\n",buf);
	fclose(fe);
	memset(filename1,0,sizeof(filename1));
	memset(filename2,0,sizeof(filename2));
	while(q != head)
	{
		if(strcmp(user.target_user,q->data.Username) == 0)
		{
			for(j = 0;j <= fdmax;j++)
			{
				if(j == q->data.sockfd)
				{
					if(write(j,buf,sizeof(buf)) == -1)
					{
						perror("private write failed");
					}
				}				
			}
		}
		q = q->next;
	}
}

static void AllTalk(int sockfd)
{
	int j;
	time_t t;
	struct tm*p;
	FILE *fp;
	char str[50],buf[100];

	t = time(NULL);
	p = localtime(&t);

	strcpy(str,asctime(p));
	strcpy(buf,"[group-talk]");
	strcat(buf," ");
	strcat(buf,str);
	strcat(buf," ");
	strcat(buf,user.Username);
	strcat(buf,"say:");
	strcat(buf,user.content);

	fp = fopen("all.txt","a+");
	fprintf(fp,"%s\n",buf);
	fclose(fp);

	for(j = 0; j <= fdmax;j++)
	{
		if(FD_ISSET(j,&master) && j != sd && j != sockfd)
		{
			if(write(j,buf,sizeof(buf)) == -1)
			{
				perror("all talk failed!:");
			}
		}
	}
}

static void check(int sockfd,char *filename)
{
	FILE *fp;
	char buf[100] = {0};
	int fd;

	fp = fopen(filename,"r");
	fseek(fp,0L,SEEK_SET);
	fgets(buf,sizeof(buf),fp);

	do
	{
		write(sockfd,buf,sizeof(buf));
		bzero(buf,sizeof(buf));
	  	fgets(buf,sizeof(buf),fp);
		if(strlen(buf) < 10)
			break;
	}while(!feof(fp));

	write(sockfd," ",sizeof(" "));
	memset(filename,0,sizeof(filename));
	fclose(fp);
}

static void update(struct node *head,int sockfd)
{
	struct node *p = head->next;
	while(p != head)
	{
		if(p->data.sockfd == sockfd)
		{
			p->data.flag = 0;
			p->data.sockfd = 0;
			break;
		}
		p = p->next;
	}
	return;
}

int main()
{
	int i,len;
	int newsd;
	int val;
	struct sockaddr_in saddr,caddr;
	socklen_t caddr_len;
	char filename[30];
	char buf[100];

	struct node *myhead = list_init();
	//选择TCP协议
	sd = socket(AF_INET,SOCK_STREAM,0);
	if(sd < 0)
	{
		perror("socket()");
		exit(1);
	}

	//SO_REUSEADDR设置端口被释放后可立即重新使用
	if(setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val)) < 0)
	{
		perror("setsockopt()");
		exit(1);
	}

	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(atoi(SERVERPORT));
	inet_pton(AF_INET,"IP",&saddr.sin_addr.s_addr);

	if(bind(sd,(void *)&saddr,sizeof(saddr)) < 0)
	{
		perror("bind()");
		exit(1);
	}

	if(listen(sd,20) < 0)
	{
		perror("listen:()");
		exit(1);
	}

	FD_ZERO(&master);
	FD_ZERO(&rset);
	FD_SET(sd,&master);
	fdmax = sd;

	while(1)
	{
		rset = master;
		if(select(fdmax+1,&rset,NULL,NULL,NULL) < 0)
		{
			if(errno == EINTR)
				continue;
			perror("select:()");
			exit(1);
		}

		caddr_len  = sizeof(caddr);
		for( i = 0 ;  i <= fdmax ; i++)
		{
			if(FD_ISSET(i,&rset))
			{
				if(i == sd)
				{
					if((newsd = accept(sd,(void *)&caddr,&caddr_len)) < 0)
					{
						if(errno == EINTR || errno == EAGAIN)
							continue;
						perror("newsd:()");
						exit(1);
					}
					else
					{
						FD_SET(newsd,&master);
						if(newsd > fdmax)
						{
							fdmax = newsd;
						}
						sprintf(buf,"Your socket ID is %d.",newsd);
					}
				}
				else
				{
					if(len = read(i,&user,sizeof(user)) <= 0)
					{
						if(errno == EINTR)
							continue;
						if(len == 0)
						{
							update(myhead,i);
							printf("SocketID %d has left !\n",i);
						}
						if(len < 0)
						{
							perror("the recv() go end!\n");
						}
						close(i);
						FD_CLR(i,&master);
					}
					else
					{
						switch(user.cmd)
						{
						case REG:
							Register(i,user);
							break;
						case LOGIN:
							Login(myhead,i,user);
							break;
						case ADD:
							Addfriend(myhead,i,user);
							break;
						case LOOK:
							Look(myhead,i,user);
							break;
						case CHOWN:
							strcpy(filename,user.Username);
							strcat(filename,user.target_user);
							strcat(filename,".txt");
							check(i,filename);
							break;
						case CHALL:
							strcpy(filename,"all.txt");
							check(i,filename);
							break;
						case ALL:
							AllTalk(i);
							break;
						case SIGLE:
							PrivateTalk(myhead,i);
							break;
						}
					}
				}
			}
		}
	}
}
		
