// This is the echo SERVER server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>

#define  MAX 256

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

int  sock, newsock;                  // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;                   // help variables

// Server initialization code:

int server_init(char *name)
{
    printf("==================== server init ======================\n");
    // get DOT name and IP address of this host

    printf("1 : get and show server host info\n");
    hp = gethostbyname(name);
    if (hp == 0)
    {
        printf("unknown host\n");
        exit(1);
    }
    printf("    hostname=%s  IP=%s\n",
           hp->h_name,  inet_ntoa(*(long *)hp->h_addr));

    //  create a TCP socket by socket() syscall
    printf("2 : create a socket\n");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("socket call failed\n");
        exit(2);
    }

    printf("3 : fill server_addr with host IP and PORT# info\n");
    // initialize the server_addr structure
    server_addr.sin_family = AF_INET;                  // for TCP/IP
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address
    server_addr.sin_port = 0;   // let kernel assign port

    printf("4 : bind socket to host info\n");
    // bind syscall: bind the socket to server_addr info
    r = bind(sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
    if (r < 0)
    {
        printf("bind failed\n");
        exit(3);
    }

    printf("5 : find out Kernel assigned PORT# and show it\n");
    // find out socket port number (assigned by kernel)
    length = sizeof(name_addr);
    r = getsockname(sock, (struct sockaddr *)&name_addr, &length);
    if (r < 0)
    {
        printf("get socketname error\n");
        exit(4);
    }

    // show port number
    serverPort = ntohs(name_addr.sin_port);   // convert to host ushort
    printf("    Port=%d\n", serverPort);

    // listen at port with a max. queue of 5 (waiting clients)
    printf("5 : server is listening ....\n");
    listen(sock, 5);
    printf("===================== init done =======================\n");
}

char* myargs[64];
char mypath[64][64];
//char input[128];
char filename[64];
char cmd[64];
main(int argc, char *argv[],char* env[])
{
    int i,j;
    char  tmp1[64];
    char  tmp2[64];
    char *hostname;
    char line[MAX];
    char* tmparg;
    char tmppath[256];
    char** myargsptr = myargs;
    char* reset;
    int resetindex = -1;
    i = 0;
    int pid,status;
    FILE * fd;
    i = 0;
    for (i=0; i<64; i++)
    {
        myargs[i] = (char*)malloc(sizeof(char)*64);

    }
    strcpy(tmppath,getenv("PATH"));
    i =0;
    tmparg = strtok(tmppath,":");
    while(tmparg!=NULL)
    {
        strcpy(mypath[i],tmparg);
        tmparg = strtok(NULL,":");
        //printf("path[%d]=%s\n",i,mypath[i]);
        i++;
    }
    if (argc < 2)
        hostname = "localhost";
    else
        hostname = argv[1];

    server_init(hostname);

    // Try to accept a client request
    while(1)
    {
        printf("server: accepting new connection ....\n");

        // Try to accept a client connection as descriptor newsock
        length = sizeof(client_addr);
        newsock = accept(sock, (struct sockaddr *)&client_addr, &length);
        if (newsock < 0)
        {
            printf("server: accept error\n");
            exit(1);
        }
        printf("server: accepted a client connection from\n");
        printf("-----------------------------------------------\n");
        printf("        IP=%s  port=%d\n", inet_ntoa(client_addr.sin_addr.s_addr),
               ntohs(client_addr.sin_port));
        printf("-----------------------------------------------\n");

        // Processing loop
        while(1)
        {
            n = read(newsock, line, MAX);
            if (n==0)
            {
                printf("server: client died, server loops\n");
                close(newsock);
                break;
            }

            // show the line string
            printf("server: read  n=%d bytes; line=[%s]\n", n, line);
            i = 0;
            if (resetindex>=0)
                myargs[resetindex]=reset;
            tmparg = strtok(line," \n");
            while (tmparg!=NULL)
            {
                strcpy(myargs[i],tmparg);
                tmparg = strtok(NULL," \n");
                //printf("args[%d]=%s\n",i,myargs[i]);
                i++;
            }
            reset = myargs[i];
            resetindex = i;
            myargs[i] = NULL;

            int pipefd[2];
            pipe(pipefd);
            if (strcmp(myargs[0],"exit")==0)
            {
                exit(0);

            }
            else if (strcmp(myargs[0],"cd")==0)
            {
                if (myargs[1]==NULL)
                {
                    chdir(getenv("HOME"));
                }
                else
                    chdir(myargs[1]);

                write(newsock,"*`*END*`*",MAX);


            }
            else if (strcmp(myargs[0],"get")==0)
            {
                if (myargs[1]==NULL)
                {
                    perror("please in a filename");
                }
                else
                {
                    char buff[MAX];
                    int fd = open(myargs[1], O_RDONLY);
                    int n;
                    if(fd < 0)
                    {
                        perror("server sending error 1\n");
                    }
                    else
                    {
                        bzero(buff,MAX);
                        while(read(fd,buff,MAX)!=0)
                        {
                            n = write(newsock, buff, MAX);
                            bzero(buff,MAX);
                        }
                        printf("Transfer complete\n");
                        write(newsock,"*`*END*`*",MAX);
                    }
                }
            }
            else if (strcmp(myargs[0],"put")==0)
            {
                if(myargs[1]==NULL)
                {
                    perror("server put error 1\n");
                }
                else
                {
                    char buff[MAX];
                    char *lastparenth = strrchr(myargs[1],'/')+1;
                    int fd = open(lastparenth, O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR|S_IRGRP);
                    int n;
                    if(fd < 0)
                    {
                        perror("get error 1\n");
                        return 1;
                    }
                    printf("in put\n");
                    while(read(newsock, buff, MAX)!=0)
                    {
                        if(strcmp(buff,"*`*END*`*")==0)
                            break;
                        n = write(fd, buff, MAX);
                        bzero(buff,MAX);
                    }
                    printf("File transfer complete\n");

                }
            }
            else
            {

                printf("Parent id=%d",getpid());
                pid = fork();
                if (pid<0)
                {
                    perror("fork failed holy shit");
                    exit(0);
                }
                if (pid) //Parent
                {
                    close(pipefd[1]);
                    pid = wait(&status);
                    printf(" Child process dead=%d, How=%04x\n",pid,status);

                }
                else //Child
                {

                    i = 0;
                    if (myargs[0][0]=='/')
                    {
                        //only exec in current directory
                        strcpy(tmppath,myargs[0]);

                        if (access( tmppath, F_OK ) != -1)
                        {
                            printf("executing for path= %s\n",tmppath);
                            close(pipefd[0]);
                            dup2(pipefd[1],1);
                            dup2(pipefd[1],2);
                            close(pipefd[1]);
                            status = execve(tmppath,myargs,env);
                        }
                        exit(status);
                    }
                    else
                    {
                        while(strcmp(mypath[i],"\0")!=0)
                        {
                            strcpy(tmppath,mypath[i]);
                            strcat(tmppath,"/");
                            strcat(tmppath,myargs[0]);

                            if (access( tmppath, F_OK ) != -1)
                            {
                                strcpy(mypath[i],tmppath);
                                printf("executing for path= %s\n",tmppath);
                                close(pipefd[0]);
                                dup2(pipefd[1],1);
                                dup2(pipefd[1],2);
                                close(pipefd[1]);
                                status = execve(tmppath,myargs,env);
                                if (status!=0)
                                    perror("exec failed:");
                            }
                            i++;
                        }
                        exit(status);


                    }


                }
                while(read(pipefd[0],line,MAX)!=0)
                {
                    n = write(newsock, line, MAX);
                    n = read(newsock, line, MAX);
                    bzero(line,MAX);
                }
                n = write(newsock,"*`*END*`*",MAX);
            }

            //shutdown(newsock, SHUT_WR);

            //printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
            printf("server: ready for next request\n");
        }
    }
}
