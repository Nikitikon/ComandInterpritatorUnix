#ifndef DF_H_INCLUDED
#define DF_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <mntent.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/statvfs.h>

typedef struct dflags {
    int k;
    int h;
}DFlags;

DFlags Flags;

typedef struct Fsize{
    char bloc[23];
    char use[23];
    char _free[23];
    char percent[5];
} FSize;


void FolderSize(char name[], FSize *f){
    struct statvfs fiData;
    int parametr = 8;
    if(statvfs(name, &fiData) < 0){
        perror(name);
    }

    if(Flags.k || Flags.h){
        parametr = 4;
    }


    long bloc = fiData.f_blocks * parametr;
    long _free = fiData.f_bavail * parametr;
    long use = (fiData.f_blocks - fiData.f_bfree) * parametr;
    int percent = (int)(use / (double)bloc * 100);

    sprintf(f->bloc, "%li", bloc);
    sprintf(f->_free, "%li", _free);
    sprintf(f->use, "%li", use);
    if(Flags.k || Flags.h){
        strcat(f->bloc, "K");
        strcat(f->use, "K");
        strcat(f->_free, "K");
    }

    if(Flags.h) {
        int count = 0;
        while(1){
            int flag = 0;
            if(bloc > 1024){
                int mod = bloc % 1024;
                bloc = bloc / 1024;
                char fbuf[5];
                sprintf(f->bloc, "%li", bloc);
                strcat(f->bloc, ".");
                sprintf(fbuf, "%li", mod);
                strcat(f->bloc, fbuf);
                if(count == 0)
                    strcat(f->bloc, "M");
                if(count == 1)
                    strcat(f->bloc, "G");
                if(count == 2)
                    strcat(f->bloc, "T");
                flag = 1;
            }


            if(use > 1024){
                int mod = use % 1024;
                use = use / 1024;
                char fbuf[5];
                sprintf(f->use, "%li", use);
                strcat(f->use, ".");
                sprintf(fbuf, "%li", mod);
                strcat(f->use, fbuf);
                if(count == 0)
                    strcat(f->use, "M");
                if(count == 1)
                    strcat(f->use, "G");
                if(count == 2)
                    strcat(f->use, "T");

                flag = 1;
            }

            if(_free > 1024){
                int mod = _free % 1024;
                _free = _free / 1024;
                char fbuf[5];
                sprintf(f->_free, "%li", _free);
                strcat(f->_free, ".");
                sprintf(fbuf, "%li", mod);
                strcat(f->_free, fbuf);
                if(count == 0)
                    strcat(f->_free, "M");
                if(count == 1)
                    strcat(f->_free, "G");
                if(count == 2)
                    strcat(f->_free, "T");

                flag = 1;
            }

            count++;
            if(!flag)
                break;
        }
    }

    sprintf(f->percent, "%li", percent);
    strcat(f->percent, "%");

}

int Filter(char mnt[]){
    int flag = 1;
    if(strcmp("sysfs", mnt) == 0)
            flag = 0;
    if(strcmp("proc", mnt) == 0)
            flag = 0;
    if(strcmp("devpts",mnt) == 0)
            flag = 0;
    if(strcmp("securityfs", mnt) == 0)
            flag = 0;
    if(strcmp("cgroup",mnt) == 0)
            flag = 0;
    if(strcmp("pstore",mnt) == 0)
            flag = 0;
    if(strcmp("mqueue",mnt) == 0)
            flag = 0;
    if(strcmp("systemd-1",mnt) == 0)
            flag = 0;
    if(strcmp("debugfs",mnt) == 0)
            flag = 0;
    if(strcmp("hugetlbfs",mnt) == 0)
            flag = 0;
    if(strcmp("fusectl",mnt) == 0)
            flag = 0;
    if(strcmp("gvfsd-fuse",mnt) == 0)
            flag = 0;
    if(strcmp("binfmt_misc",mnt) == 0)
            flag = 0;

    return flag;
}


#endif // DF_H_INCLUDED
