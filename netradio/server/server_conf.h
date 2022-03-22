#ifndef SERVER_CONF_H__
#define SERVER_CONF_H__

#define DEFAULT_MEDIADIR   "/var/media/"
#define DEFAULT_IF         "ens33"

enum
{
	RUN_DAEMON = 1,
	RUN_FOREGROUND
};

struct server_conf_st
{
	char *rcvport;
	char *mgroup;
	char *media_dir;  //媒体库
	char runmode;     //运行模式，位图(前台运行/守护)
	char *ifname;     //指定网卡	
};

extern struct server_conf_st server_conf;
extern int serversd;
extern struct sockaddr_in sndaddr;

#endif
