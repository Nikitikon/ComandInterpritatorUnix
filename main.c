#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <getopt.h>
#include <fcntl.h>

#include "ps.h"
#include "link.h"
#include "df.h"
#include "cp.h"


int shell_active = 1; // требуется для команды exit

//  команды оформлены в виде макросов
//  доп. информация  HYPERLINK "http://en.wikipedia.org/wiki/C_preprocessor" http://en.wikipedia.org/wiki/C_preprocessor

#define SHCMD(x) int shcmd_##x (char* cmd, char* params[])
#define SHCMD_EXEC(x) shcmd_##x (params[0], params)
#define IS_CMD(x) strncmp(#x,cmd,strlen(cmd)) == 0
#define LINELENGHT 1024
int paramscount;

// Встроенные команды
// - pwd, exit

SHCMD(pwd)
{
    printf("%s\n",getenv("PWD"));
    return 0;
}

SHCMD(exit)
{
    shell_active = 0;
    printf("Bye, bye!\n");
    return 0;
}

SHCMD(ls)
{
    char flag_l = 0, flag_a = 0, flag_i = 0;
    int opt = getopt(paramscount, params, "alih");
    while (opt != -1) {
        switch (opt) {
            case 'a':
                flag_a = 1;
                break;

            case 'l':
                flag_l = 1;
                break;

            case 'i':
                flag_i = 1;
                break;

            case 'h':
                printf("Справочка по ls\n");
                return 0;
                break;

            default:
                break;
        }
        opt = getopt(paramscount, params, "alih");
    }

    struct dirent * de;
    struct dirent ** des;

    int files_count;
    files_count = scandir(".", &des, NULL, alphasort);

    for (int i = 0; i < files_count; i++) {
        de = des[i];
        if (!flag_a && de->d_name[0] == '.') {
            continue;
        }

        struct stat st;
        lstat(de->d_name, &st);

        if (flag_i) {
            printf("%llu ", st.st_ino);
        }

        if (!flag_l) {
            printf("%-6s ", de->d_name);
        }
        else {
            int mode = st.st_mode;
            static char * rights = "rwxrwxrwx";
            if (S_ISDIR(mode))
                printf("d");
            else
                printf("-");
            for (int j = 0; j < 9; j++) {
                printf(mode&256 ? "&c" : "-", rights[0]);
                mode <<= 1;
            }
            printf(" %3d", st.st_nlink);
            printf(" %7s", getpwuid(st.st_uid)->pw_name);
            printf(" %7s", getgrgid(st.st_gid)->gr_name);
            printf(" %10lld", st.st_size);
            char buffer[80];
            strftime(buffer, 80, "%Y-%m-%d %H:%M", localtime(&st.st_mtime));
            printf(" %s", buffer);
            printf(" %s", de->d_name);
            printf("\n");
        }
        free(des[i]);
    }
    free(des);
    printf("\n");
    return 0;
}

SHCMD(cd)
{
    if (paramscount == 2) {
        if (params[1][0] == '/') {
            if (chdir(params[1]) == 0)
                setenv("PWD", params[1], 1);
            else
                printf("Ошибка изменения пути\n");
        }
        else{
            char* CurentDir = getenv("PWD");
            char NewDir[256] = "";
            int NewDirLenght = 1 + strlen(params[1]) + strlen(CurentDir);
            if (strcmp(params[1], "..") == 0) {
                char * pch;
                pch = strrchr(CurentDir, '/');
                CurentDir[pch - CurentDir] = '\0';
                strcpy(NewDir, CurentDir);
            }
            else{
                snprintf(NewDir, NewDirLenght + 1, "%s/%s\0", CurentDir, params[1]);
            }

            if (chdir(NewDir) == 0)
                setenv("PWD", NewDir, 1);
            else
                printf("Ошибка изменения пути");
        }
    }

    return 0;
}

SHCMD(ps){
    struct proc_t ps;

	DIR *Dir = opendir(PROC_FS);

	char time[256];

	printf("UID\t  PID  PPID  C\tSTIME\tTTY\tTIME\tCMD\n");
	while ((entry = readdir(Dir))) {
		if(!isdigit(*entry->d_name))
			continue;
		read_proc_stat(&ps,entry->d_name);
		printf("%s\t",getpwuid(ps.uid)->pw_name);
		printf("%5d ",ps.pid);
		printf("%5d ",ps.ppid);
		printf("%2d\t",ps.nice);//?

		strftime(time, sizeof(time), "%H:%M", localtime(&ps.ctime));
		printf("%s\t",time);

		printf("\t");
		printf("\t");
		printf("%s\t",ps.cmd);
		printf("\n");
	}

	closedir(Dir);
    return 0;
}

SHCMD(link){
    int flags;
    flags = 0;
    int opt;
    while ((opt = getopt(paramscount, params, "s")) != -1)
    {
        switch(opt)
        {
        case 's':
            flags = 1;
        default:
            break;
        }
    }
    if (flags == 0)
    {
        if (link(params[optind], params[optind+1]) == -1)
        {
            perror("link");
            return -1;
        }
    }
    else
    {
        if(symlink(params[optind], params[optind+1]) == -1)
        {
            perror("symlink");
            return -1;
        }
    }

    return 0;
}

SHCMD(df){
    Flags.h = 0;
    Flags.k = 0;
    int opt = getopt(paramscount, params, "kh");
    while (opt != -1) {
        switch (opt) {
            case 'k':
                Flags.k = 1;
                break;
            case 'h':
                Flags.h = 1;
                break;

            default:
                break;
        }
        opt = getopt(paramscount, params, "kh");
    }
    struct mntent *ent;

    FILE *aFile;

    aFile = setmntent("/etc/mtab", "r");
    if(aFile == NULL){
        perror("setmntent");
        exit(1);
    }
    printf("File System Mount Point All Memory Use Memory Free Memory Percent\n");
    while((ent = getmntent(aFile)) != NULL){
        if(Filter(ent->mnt_fsname)){
            FSize f;
            FolderSize(ent->mnt_dir, &f);
            printf("%10.10s  %10.10s  %10.10s  %10.10s  %10.10s  %7.7s\n", ent->mnt_fsname, ent->mnt_dir, f.bloc, f.use, f._free, f.percent);
        }
    }

    endmntent(aFile);
    return 0;
}

SHCMD(head){

    int FlagN = 0;
    int FlagC = 0;

    if (paramscount<2)
    {
        printf("Too few arguments! \n");
    }

    int c;
    int intArg;

    while ((c = getopt(paramscount, params, "n:c:"))!=-1) {
        intArg = atoi(optarg);

        switch (c) {

        case 'n':
            FlagN = 1;
            break;
        case 'c':
            FlagC = 1;
            break;
        default:
            printf ("Error!\n", c);
        }
    }

    if (!FlagC && !FlagN)
    {
        FlagN = 1;
        intArg = 10;
    }

    if (optind < paramscount) {
        while (optind < paramscount)
        {
            char* fileName = params[optind++];

            FILE* file = fopen(fileName, "r");


            if (!file)
            {
                printf("Can't open file!\n");
                return 0;
            }


            if (FlagN)
            {
                char str [LINELENGHT];

                int i=0;
                while(fgets(str,sizeof(str),file) && i++ < intArg)
                    printf ("%s", str);

                fclose (file);
                return 0;
            }

            if (FlagC)
            {

                int i;
                for (i = 0; i<intArg; i++)
                {
                    int symbol = getc (file);
                    if (symbol != EOF)
                        printf("%c", symbol);
                    else
                        break;
                }

                return 0;
            }
        }


}
    return 0;
}


SHCMD(cp){
    if(paramscount < 3){
        printf("Too few argument");
        return 0;
    }

    CpFlags.manyFlag = 0;
    CpFlags.oneFlag = 0;

    if(paramscount == 3)
        CpFlags.oneFlag = 1;

    if(paramscount > 3)
        CpFlags.manyFlag = 1;


    if(CpFlags.oneFlag){
        printf("Copy");
        int rs = CopyFile(params[1], params[2]);
        if(rs == -1){
            printf("UPS");
        }
        return 0;
    }

    if(CpFlags.manyFlag){
        for(int i = 1; i < paramscount - 1; i++){
            char fileName[20];
            memset(fileName, '\0', 20);
            int j = strlen(params[i]) - 1;
            while(params[i][j] != '/' && j >=0){
                fileName[j] = params[i][j];
                j--;
            }

            char buf[50];
            memset(buf, '\0', 50);
            strcat(buf, params[paramscount - 1]);
            strcat(buf, fileName);

            int rs = CopyFile(params[i], buf);
            if(rs == -1){
            printf("UPS");}
        }

        return 0;
    }
    return 0;
}

void my_exec(char *cmd)
{
    char *params[256]; //параметры команды разделенные пробелами
    char *token;
    int np;

    token = strtok(cmd, " ");
    np = 0;
    while( token && np < 255 )
    {
        params[np++] = token;
        token = strtok(NULL, " ");
    }
    params[np] = NULL;
    paramscount = np;

    // выполнение встроенных команд
    if( IS_CMD(pwd) )
        SHCMD_EXEC(pwd);
    else
        if( IS_CMD(exit) )
            SHCMD_EXEC(exit);
        else
            if (IS_CMD(ls))
                SHCMD_EXEC(ls);
            else
                if (IS_CMD(cd))
                    SHCMD_EXEC(cd);
                else
                    if (IS_CMD(ps))
                        SHCMD_EXEC(ps);
                    else
                        if(IS_CMD(link))
                            SHCMD_EXEC(link);
                        else
                            if(IS_CMD(df))
                                SHCMD_EXEC(df);
                            else
                                if(IS_CMD(head))
                                SHCMD_EXEC(head);
                                    else
                                        if(IS_CMD(cp))
                                            SHCMD_EXEC(cp);
                                        else
                    { // иначе вызов внешней команды
                        execvp(params[0], params);
                        perror("exec"); // если возникла ошибка при запуске
                    }
}
// рекурсивная функция обработки конвейера
// параметры: строка команды, количество команд в конвейере, текущая (начинается с 0 )
int exec_conv(char *cmds[], int n, int curr)
{
    int fd[2],i;
    if( pipe(fd) < 0 )
    {
        printf("Cannot create pipe\n");
        return 1;
    }

    if( n > 1 && curr < n - 2 )
    { // first n-2 cmds
        if( fork() == 0 )
        {
            dup2(fd[1], 1);
            close(fd[0]);
            close(fd[1]);
            my_exec(cmds[curr]);
            exit(0);
        }
        if( fork() == 0 )
        {
            dup2(fd[0], 0);
            close(fd[0]);
            close(fd[1]);
            exec_conv(cmds,n,++curr);
            exit(0);
        }
    }
    else
    { // 2 last cmds or if only 1 cmd
        if( n == 1 && (strcmp(cmds[0],"exit") == 0 ))
        { // для случая команды exit обходимся без fork, иначе не сможем завершиться
            close(fd[0]);
            close(fd[1]);
            my_exec(cmds[curr]);
            return 0;
        }
        if (strncmp(cmds[0],"cd", 2) == 0 && n == 1) {
            close(fd[0]);
            close(fd[1]);
            my_exec(cmds[curr]);
            return 0;
        }
        if( fork() == 0 )
        {
            if( n > 1 ) // if more then 1 cmd
                dup2(fd[1], 1);
            close(fd[0]);
            close(fd[1]);
            my_exec(cmds[curr]);
            exit(0);
        }
        if( n > 1 && fork()==0 )
        {
            dup2(fd[0], 0);
            close(fd[0]);
            close(fd[1]);
            my_exec(cmds[curr+1]);
            exit(0);
        }
    }
    close(fd[0]);
    close(fd[1]);

    for( i = 0 ; i < n ; i ++ ) // ожидание завершения всех дочерних процессов
        wait(0);

    return 0;
}

void Do (char* command)
{
    char *p, *token;
    int cmd_cnt;
    char *cmds[256];

    if( (p = strstr(command,"\n")) != NULL ) p[0] = 0;

    token = strtok(command, "|");
	for(cmd_cnt = 0; token && cmd_cnt < 256; cmd_cnt++ )
	{
		cmds[cmd_cnt] = strdup(token);
		token = strtok(NULL, "|");
	}
	cmds[cmd_cnt] = NULL;

	if( cmd_cnt > 0 )
	{
		exec_conv(cmds,cmd_cnt,0);
	}
}

// главная функция, цикл ввода строк (разбивка конвейера, запуск команды)
int main(int argc, char** argv)
{
    if (argc>1)
    {
        FILE* file = fopen(argv[1], "r");
        if (!file)
        {
            printf("Can't open file!\n");
            return 0;
        }

        char str [LINELENGHT];

        while(fgets(str,sizeof(str),file))
        {
            Do (str);
        }
    }
    else
    {

    char cmdline[1024];
    char *p, *cmds[256], *token;
    int cmd_cnt;

    while( shell_active )
    {
        printf("[%s]# ",getenv("PWD"));
        fflush(stdout);

        fgets(cmdline,1024,stdin);
        if( (p = strstr(cmdline,"\n")) != NULL )
            p[0] = 0;

        token = strtok(cmdline, "|");
        for(cmd_cnt = 0; token && cmd_cnt < 256; cmd_cnt++ )
        {
            cmds[cmd_cnt] = strdup(token);
            token = strtok(NULL, "|");
        }
        cmds[cmd_cnt] = NULL;

        if( cmd_cnt > 0 )
        {
            exec_conv(cmds,cmd_cnt,0);
        }
    }
    }
    return 0;
}
