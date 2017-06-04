#ifndef CP_H_INCLUDED
#define CP_H_INCLUDED

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct flagcp{
    int oneFlag;
    int manyFlag;
    int dirflag;
} CFlags;

CFlags CpFlags;

int CopyFile(const char* source, const char* destination)
{
    int input, output;
    if ((input = open(source, O_RDONLY)) == -1)
    {
        return -1;
    }
    if ((output = open(destination, O_RDWR | O_CREAT)) == -1)
    {
        close(input);
        return -1;
    }

    off_t bytesCopied = 0;
    struct stat fileinfo = {0};
    fstat(input, &fileinfo);
    int result = sendfile(output, input, &bytesCopied, fileinfo.st_size);

    char mode[] = "0644";
    int i;
    i = strtol(mode, 0, 8);
    chmod (destination,i);

    close(input);
    close(output);

    return result;
}

#endif // CP_H_INCLUDED
