#ifndef CLIENT_H__
#define CLIENT_H__

#define DEFAULT_PLAYERCMD	"/usr/bin/mpg123 - > /dev/null" //加-表示只播放标准输入的内容

struct client_conf_st
{
	char *rcvport;
	char *mgroup;
	char *player_cmd;
};

extern struct client_conf_st client_conf;

#endif
