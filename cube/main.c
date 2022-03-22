#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>

#include "draw.h"


extern int max;
extern int grade;

int main()
{
	max = 0,grade = 0;
	echo_cancel();
	setscreen();
	readscore();
	block_init();
	srand((unsigned int)time(NULL));
	startgame();
	exit(0);
}
