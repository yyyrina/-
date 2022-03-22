#ifndef DRAW_H__
#define DRAW_H__

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define row_begin 3
#define row_end   19
#define col_begin 4
#define col_end   15
#define tmp "/home/yyyrina/cyy/first/cube/my_tetris/maxscore.txt"
/*
#define DOWN  80
#define LEFT  75
#define RIGHT 77
#define UP    72
*/

void echo_cancel(void);
void echo_restore(void);
void setpos(int ,int );
void setscreen(void);
void block_init(void);
void block_print(int ,int ,int ,int );
void block_clear(int ,int ,int ,int );
int islegal(int shape,int form,int x,int y);
int isdisapear(void);
void startgame(void);
int isover(void);
void readscore(void);
void writescore(void);

#endif
