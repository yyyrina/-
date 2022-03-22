#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#include "draw.h"

struct face_st
{
	int data[row_end+1][col_end+1];
	int color[row_end+3][col_end+3];
}face;

struct block
{
	int space[4][4];
}block[7][4];

int grade = 0;
static int old_fcntl;
static struct termios old;
static int stop = 0;
int max = 0;
static int shape1,form1;
static int x1,y1;

//设置属性
void echo_cancel(void)
{
	//无回显
	struct termios new;
	tcgetattr(STDIN_FILENO,&old);
	tcgetattr(STDIN_FILENO,&new);
	new.c_lflag &= ~(ECHO | ICANON);
	new.c_cc[VMIN] = 1;
	new.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO,TCSANOW,&new);

	//无阻塞
	old_fcntl = fcntl(STDIN_FILENO,F_GETFL);
	fcntl(STDIN_FILENO,F_SETFL,old_fcntl | O_NONBLOCK);

	//无光标
	printf("\033[?25l");

	//清屏
	printf("\033[2J");

}

//恢复属性
void echo_restore(void)
{
	fcntl(STDIN_FILENO,F_SETFL,old_fcntl);
	tcsetattr(STDIN_FILENO,TCSANOW,&old);
	setpos(col_end,row_end);
	printf("\n\n");
	printf("\033[?25h");
	printf("\033[0m");
}

//设置坐标
void setpos(int x,int y)
{
	printf("\033[%d;%dH",y+1,x+1);
}

//初始屏幕显示
void setscreen(void)
{
	int i,j;

	setpos(0,0);
	for(i = row_begin ; i <= row_end ; i++)
	{
		for(j = col_begin ; j <= col_end ; j++)
		{
			if(i == row_begin || i == row_end )
			{
				face.data[i][j] = 1;//标记该位置有方块
				setpos(2*j,i);
				printf("\033[30;45m==");
			}
			else if(j == col_begin || j == col_end)
			{
				face.data[i][j] = 1;
				setpos(2*j,i);		
				printf("\033[30;45m||");
			}
			else
				face.data[i][j] = 0;
		}
	}

	setpos(2*col_end+8,row_begin);
	printf("\033[30;47m下一个方块");

	setpos(2*col_end+8,(row_begin+row_end)/2);
	printf("\033[30;47mscore");

	setpos(2*col_end+10,(row_begin+row_end)/2+1);
	printf("\033[30;47m%d",grade);

	setpos(2*col_end+8,(row_begin+row_end)/2+7);
	printf("\033[30;47m状态");

	setpos(2*col_end +8,(row_begin+row_end)/2+8);
	printf("\033[30;47mplaying");

}

//存放方块
void block_init(void)
{
	int form;
	int shape;
	int temp[4][4];
	int i,j;
	//T
	for(i = 0 ; i <= 2 ; i++)
		block[0][0].space[1][i] = 1;
	block[0][0].space[2][1] = 1;

	//L
	for(i = 1 ; i <= 3; i++)
		block[1][0].space[i][1] = 1;
	block[1][0].space[3][2] = 1;

	//J
	for(i = 1 ; i <= 3; i++)
		block[2][0].space[i][2] = 1;
	block[2][0].space[3][1] = 1;

	for(i = 0;i <= 1; i++)
	{
		//Z
		block[3][0].space[1][i] = 1;
		block[3][0].space[2][i+1] = 1;

		//S
		block[4][0].space[1][i+1] = 1;
		block[4][0].space[2][i] = 1;

		//O
		block[5][0].space[1][i+1] = 1;
		block[5][0].space[2][i+1] = 1;
	}

	//I
	for(i = 0;i <= 3;i++)
		block[6][0].space[i][1] = 1;

	for(shape = 0;shape < 7;shape++)//七种形状
	{
		for(form = 0;form < 3;form++)//四种形态
		{
			//获取第form种形态
			for(i = 0;i < 4;i++)
			{
				for(j = 0;j < 4 ; j++)
				{
					temp[i][j] = block[shape][form].space[i][j];
				}
			}
	
			for(i = 0 ; i < 4 ; i++)
			{
				for(j = 0 ;j < 4 ;j++)
				{
					block[shape][form+1].space[i][j] = temp[3-j][i];
				}
			}
		}
	}
}

//输出方块
void block_print(int shape ,int form , int x, int y)
{
	int i,j;
	for(i = 0;i < 4; i++)
	{
		for(j = 0;j < 4;j++)
		{
			if(block[shape][form].space[i][j] == 1)
			{
			
				switch(shape)
				{
					case 0:
						setpos(2*(x+j),y+i);
						printf("\033[30;41m[]");
						break;
					
					case 1:
						setpos(2*(x+j),y+i);
						printf("\033[30;42m[]");
						break;

					case 2:
						setpos(2*(x+j),y+i);
						printf("\033[30;43m[]");
						break;

					case 3:
						setpos(2*(x+j),y+i);
						printf("\033[30;44m[]");
						break;

					case 4:
						setpos(2*(x+j),y+i);
						printf("\033[30;45m[]");
						break;

					case 5:
						setpos(2*(x+j),y+i);
						printf("\033[30;46m[]");
						break;

					case 6:
						setpos(2*(x+j),y+i);
						printf("\033[30;41m[]");
						break;
				}
		      
			}
		}
	}
}

//消除方块
void block_clear(int shape,int form,int x,int y)
{
	int i;
	int j;

	for(i = 0; i < 4;i++)
	{
		for(j = 0 ;j < 4; j++)
		{
			if(block[shape][form].space[i][j] == 1)
			{
				setpos(2*(x+j),y+i);
				printf("\033[0m  ");

			}
		}
	}
}

//合法性判断
int islegal(int shape,int form,int x,int y)
{
	int i,j;
	for(i = 0;i < 4; i++)
	{
		for(j = 0;j < 4;j++)
		{
			if((block[shape][form].space[i][j] == 1) && (face.data[y+i][x+j]) == 1)
				return -1;
		}
	}
	return 0;
}
					
//判断得分，消行
int isdisapear(void)
{
	int i,j;
	int m,n;
	int sum;
	for( i = row_end -1 ; i > row_begin ;i--)
	{
		sum = 0;
		for( j = col_begin+1; j < col_end; j++)
		{
			sum += face.data[i][j];
		}
		if(sum == 0)
			break;
		else if(sum == col_end - col_begin -1)
		{
			grade += 10;
			setpos(2*col_end+10,(row_begin+row_end)/2+1);
			printf("\033[30;47m%d",grade);
		

		for(j = col_begin+1 ; j < col_end ; j++)
		{
			face.data[i][j] = 0;
			setpos(2*j,i);
		    printf("\033[0m  ");
		}
	

	//上一行往下落
		for( m = i ; m > row_begin; m--)
		{
			sum = 0;
			for(n = col_begin+1 ; n < col_end ; n++)
			{
				sum += face.data[m-1][n];
				face.data[m][n] = face.data[m-1][n];
				face.color[m][n] = face.color[m-1][n];
				if(face.data[m][n] == 1)
				{
					setpos(2*n,m);
					switch(face.color[m][n])
					{
						case 0:
							printf("\033[30;41m[]");
							break;
						case 1:
							printf("\033[30;42m[]");
							break;
						case 2:
							printf("\033[30;43m[]");
							break;
						case 3:
							printf("\033[30;44m[]");
							break;
						case 4:
							printf("\033[30;45m[]");
							break;
						case 5:
							printf("\033[30;46m[]");
							break;
						case 6:
							printf("\033[30;41m[]");
							break;
					}
				}
				else
				{
					setpos(2*n,m);
					printf("\033[0m  ");
				}
			}	
			if (sum == 0)
				return 1;
		}
	}

	}
	return 2;
}

void readscore(void)
{
	FILE * fp;
	fp = fopen(tmp,"r+");
	if(fp == NULL)
	{
		perror("fopen:()");
		exit(1);
	}

	fread(&max,sizeof(int),1,fp);
	fclose(fp);
}

void writescore(void)
{
	FILE * fp;
	fp = fopen(tmp,"w+");
	if(fp == NULL)
	{
		perror("fopen()");
		exit(1);
	}

	fseek(fp,0,SEEK_SET);
	fwrite(&grade,sizeof(int),1,fp);
	fclose(fp);
}

//判断游戏结束
int isover(void)
{
	int j;
	char c;

	for(j = col_begin+1 ; j < col_end ; j++)
	{
		if(face.data[row_begin+2][j] == 1)
		{
			setpos(2*col_end+8,row_begin+1);
			printf("\033[0m          ");
			setpos(2*col_end+8,row_begin+2);
			printf("\033[0m          ");
			setpos(2*col_end+8,row_begin+3);
			printf("\033[0m           ");
			setpos(2*col_end+8,row_begin+4);
			printf("\033[0m           ");

			usleep(1000*100);
			printf("\033[0m");
			setscreen();
			setpos(2*col_begin+3,row_begin+(row_end-row_begin)/2);
			if(grade > max)
			{
				printf("\033[30;46m打破记录！最高记录更新为%d",grade);
				writescore();
			}
			else if(grade == max)
			{	
				printf("\033[30;46m与最高记录持平，继续加油");
			}
			else
			{
				printf("\033[30;46m小垃圾，还差%d呢，哼",max-grade);
			}

			setpos(2*col_end +8,(row_begin+row_end)/2+8);
			printf("\033[30;47mgame over!");
			while(1)
			{
				setpos(2*col_begin+4,row_begin+(row_end-row_begin)/2+2);
				printf("\033[30;46m退出?(y/n)");
				c = getchar();
				if(c == 'y')
					return 1;
			}
		}
	}
	return 2;
}

void alrm_handler(int s)
{
	int i,j;
	if(islegal(shape1,form1,x1,y1+1) == -1)
	{
		for(i = 0;i < 4;i++)
		{
			for(j = 0; j < 4 ; j++)
			{
				if(block[shape1][form1].space[i][j] == 1)
				{
					face.data[y1+i][x1+j] = 1;
					face.color[y1+i][x1+j] = shape1;
				}
			}
		}
		stop = 1;
		while(isover() == 1)
		{
			echo_restore();
			exit(0);
		}
		while((isover() == 2 && isdisapear() == 2) == 0);
	}
    else
	{
		block_clear(shape1,form1,x1,y1);
		y1++;
		block_print(shape1,form1,x1,y1);
	}		
}


void startgame(void)
{
	char ch;
	struct sigaction sa;
	struct itimerval new;

	shape1 = rand()%7,form1 = rand()%4;

	sa.sa_handler = alrm_handler;
	sa.sa_flags = 0;
	sigaction(SIGALRM,&sa,NULL);

	new.it_value.tv_sec = 0;
	new.it_value.tv_usec = 1000*500;
	new.it_interval.tv_sec = 0;
	new.it_interval.tv_usec = 1000*500;
	setitimer(ITIMER_REAL,&new,NULL);

	while(1)
	{
		x1 = (col_end-col_begin)/2+3;
		y1 = row_begin+1;
		stop = 0;
		int nextshape = rand()%7,nextform = rand()%4;
		block_print(nextshape,nextform,col_end+5,row_begin+1);

		while(!stop)
		{
			ch = getchar();
			switch(ch)
			{
				case 's':
					if(islegal(shape1,form1,x1,y1+1) == 0)
					{
						block_clear(shape1,form1,x1,y1);
						y1++;
						block_print(shape1,form1,x1,y1);
					}
					break;
				case 'a':
					if(islegal(shape1,form1,x1-1,y1) == 0)
					{
						block_clear(shape1,form1,x1,y1);
						x1--;
						block_print(shape1,form1,x1,y1);
					}
					break;
				case 'd':
					if(islegal(shape1,form1,x1+1,y1) == 0)
					{
						block_clear(shape1,form1,x1,y1);
						x1++;
						block_print(shape1,form1,x1,y1);
					}
					break;
				case 'w':
					if(islegal(shape1,(form1+1)%4,x1,y1) == 0)
					{
						block_clear(shape1,form1,x1,y1);
						form1 = (form1+1)%4;;
						block_print(shape1,form1,x1,y1);
					}
					break;
				case 'f':
					if(islegal(shape1,form1,x1,y1-1) == 0)
					{
						block_clear(shape1,form1,x1,y1);
						y1--;
						block_print(shape1,form1,x1,y1);
					}
					break;
				case 'q':
					exit(0);
			}
		}

		shape1 = nextshape,form1 = nextform;
		block_clear(nextshape,nextform,col_end+5,row_begin+1);
	}

}




