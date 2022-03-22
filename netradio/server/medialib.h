#ifndef MEDIALIB_H__
#define MEDIALIB_H__

#include "size_type.h"

#define MP3_PARTERN "/*.mp3"
#define DESC_FNAME  "/desc.txt"

#define MP3_BITRATE 	(128 * 1024)

struct mlib_listentry_st
{
	chnid_t chnid;
	char *desc;
};

int mlib_getchnlist(struct mlib_listentry_st **,int *);

int mlib_freechnlist(struct mlib_listentry_st *);

ssize_t mlib_readchn(chnid_t,void *,size_t );

#endif
