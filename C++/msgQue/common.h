#ifndef _COMMON_H_
#define _COMMON_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <cstring>

#define MSG_FILE "/home/pengzhen/.bashrc"
#define BUFFER_SIZE 1024
#define PERMISSIONS S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP

typedef struct
{
	long mtype;
	char mtext[BUFFER_SIZE];
}my_msgBuf;

#endif
