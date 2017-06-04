#ifndef PS_H_INCLUDED
#define PS_H_INCLUDED

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pwd.h>

#define PROC_FS "/proc"

struct dirent *entry;

typedef struct proc_t
{ // http://www.protocols.ru/modules.php?name=News&file=article&sid=57
	int pid;
	char cmd[256];
	unsigned char state;
	int ppid;
	int pgrp;
	int session;
	int tty_nr;
	int tpgid;
	uint flags;
	uint minflt;
	uint cminflt;
	uint majflt;
	uint cmajflt;
	int utime;
	int stime;
	int cutime;
	int cstime;
	int priority;
	uint nice;
	int zero;
	uint itrealvalue;
	int starttime;
	uint vsize;
	uint rss;
	uint rlim;
	uint startcode;
	uint endcode;
	uint startstack;
	uint kstkesp;
	uint kstkeip;
	int signal;
	int blocked;
	int sigignore;
	int sigcatch;
	uint wchan;
//additional
	int uid;
	time_t ctime;
} proc_t;

void read_proc_stat(struct proc_t *ps, char* fname)
{
	char path[256];

	struct stat st;

	sprintf(path,"/%s/%s/stat", PROC_FS, fname);

	FILE *file = fopen(path,"r");
	fscanf(file,"%d %s %c %d %d %d %d %u %u %u %u %u %d %d %d %d %d %u %d %u %d %u %u %u %u %u %u %u %u %d %d %d %u",
		&ps->pid, ps->cmd, &ps->state, &ps->ppid, &ps->pgrp, &ps->session,
		&ps->tty_nr, &ps->tpgid, &ps->flags, &ps->minflt, &ps->cminflt,
		&ps->majflt, &ps->cmajflt, &ps->utime, &ps->stime, &ps->cutime,
		&ps->cstime, &ps->priority, &ps->nice, &ps->zero, &ps->itrealvalue,
		&ps->starttime, &ps->vsize, &ps->rss, &ps->rlim, &ps->startcode,
		&ps->endcode, &ps->startstack, &ps->kstkesp, &ps->kstkeip, &ps->signal,
		&ps->blocked, &ps->sigignore, &ps->sigcatch, &ps->wchan
	);

	sprintf(path,"/%s/%s", PROC_FS, fname);
	lstat(path,&st);

	ps->uid = st.st_uid;
	ps->ctime = st.st_ctime;
	fclose(file);
}

#endif // PS_H_INCLUDED
