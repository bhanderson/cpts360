#ifndef __put_h__
#define __put_h__


#define MAX_SIZE 256

int put(char * filename, int sock )
{
    char buff[MAX_SIZE];
    int fd = open(filename, O_RDONLY);
    int n;
    if(fd < 0)
    {
        printf("put error 1\n");
        return 1;
    }
    while(read(fd,buff,MAX_SIZE)!=0)
    {
        n = write(2, buff, MAX_SIZE);
        bzero(buff,MAX_SIZE);
    }
    printf("DONE");
    return 0;
}

int get(int sock, char * filename )
{
    char buff[MAX_SIZE];
    int fd = open(filename, O_CREAT|O_WRONLY,S_IRUSR|S_IWRITE|S_IRGRP);
    int n;
    if(fd < 0)
    {
        perror("get error 1\n");
        return 1;
    }
    while(read(sock, buff, MAX_SIZE)!=0)
    {
        n = write(fd, buff, MAX_SIZE);
        bzero(buff,MAX_SIZE);
    }
    return 0;
}

#endif
