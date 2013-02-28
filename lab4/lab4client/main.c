//**************************** ECHO CLIENT CODE **************************
// The echo client client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX 256

// Define variables
struct hostent *hp;
struct sockaddr_in  server_addr;

int sock, r;
int SERVER_IP, SERVER_PORT;

int execute(char input[MAX], char **envp)
{
    char args[64][64];
    char command[64];
    char **myargv=args;
    int i = 0,filestyle=0;
    char *token = strtok(input, " \n")+1; // +1 to remove the l
    printf("token: %s\n",token);
    strcpy(command, token);

    while(token!=NULL)
    {
        myargv[i] = token;
        i++;
        token = strtok(NULL, " \n");
    }
    if(strcmp(args[0],"put")==0)
    {
        char buff[MAX];
        int fd = open(args[1], O_RDONLY);
        int n;
        if(fd < 0)
        {
            printf("put error 1\n");
            return 1;
        }
        while(read(fd,buff,MAX)!=0)
        {
            n = write(sock, buff, MAX);
            bzero(buff,MAX);
        }
        printf("DONE\n");
        write(sock,"*`*END*`*",MAX);
        return 0;
    }
    if(strcmp(args[0],"get")==0)
    {
        char buff[MAX];
        int fd = open(args[0], O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR|S_IRGRP);
        int n;
        if(fd < 0)
        {
            perror("get error 1\n");
            return 1;
        }
        while(read(sock, buff, MAX)!=0)
        {
            if(strcmp(buff,"*`*END*`*")==0)
                    break;
            n = write(fd, buff, MAX);
            bzero(buff,MAX);
        }
        return 0;
    }
    myargv[i]=NULL;
    if(strncmp(command, "cd",64)==0)
    {
        if(myargv[1]==NULL)
        {
            chdir(getenv("HOME"));
            //printf("CWD:%s\n",get_current_dir_name());
        }
        else
        {
            if(chdir(myargv[1])==-1)
                printf("chdir error\n");
            printf("CWD:%s\n",get_current_dir_name());
        }
    }
    char path[32][32] = {"/usr/local/sbin/ ","/usr/local/bin/",
                         "/usr/sbin/","/usr/bin/","/sbin/",
                         "/usr/games/","/usr/local/games","/bin/"
                        };
    pid_t pid;
    int status=1;
    //char *envp[] = { NULL };
    for(i=0; i<8&&status!=0; i++)
    {
        myargv[0] = strcat(path[i],command);

        switch (pid = fork())
        {
        case -1:
            perror("fork()");
            exit(EXIT_FAILURE);
        case 0:
            status = execve(myargv[0],myargv,envp);
            exit(status);
        default:
            if( waitpid(pid, &status, 0) < 0)
            {
                perror("waitpid()");
                exit(EXIT_FAILURE);
            }
            if( WIFEXITED(status))
            {
                if(status != 65280)
                    printf("child exit status code: %d\n",status);
                break; // do not exit get next input
            }
            exit(EXIT_FAILURE); // should not ever get here
        }
    }


}


// clinet initialization code
int client_init(char *argv[])
{
    printf("======= clinet init ==========\n");

    printf("1 : get server info\n");
    hp = gethostbyname(argv[1]);
    if (hp==0)
    {
        printf("unknown host %s\n", argv[1]);
        exit(1);
    }

    SERVER_IP   = *(long *)hp->h_addr;
    SERVER_PORT = atoi(argv[2]);

    printf("2 : create a TCP socket\n");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock<0)
    {
        printf("socket call failed\n");
        exit(2);
    }

    printf("3 : fill server_addr with server's IP and PORT#\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = SERVER_IP;
    server_addr.sin_port = htons(SERVER_PORT);

    // Connect to server
    printf("4 : connecting to server ....\n");
    r = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (r < 0)
    {
        printf("connect failed\n");
        exit(1);
    }

    printf("5 : connected OK to \007\n");
    printf("---------------------------------------------------------\n");
    printf("hostname=%s  IP=%s  PORT=%d\n",
           hp->h_name, inet_ntoa(SERVER_IP), SERVER_PORT);
    printf("---------------------------------------------------------\n");

    printf("========= init done ==========\n");
}
char* myargs[64];

char mypath[64][64];
char filename[64];
char cmd[64];
main(int argc, char *argv[ ], char *env[ ])
{
    int n;
    char line[MAX], ans[MAX];
    char* tmparg;
    char tmppath[256];
    char** myargsptr = myargs;
    char* reset;
    int resetindex = -1;
    int i = 0;
    int pid,status;
    FILE * fd;
    i = 0;
    for (i=0; i<64; i++)
    {
        myargs[i] = (char*)malloc(sizeof(char)*64);

    }
    strcpy(tmppath,getenv("PATH"));

    if (argc < 3)
    {
        printf("Usage : client ServerName SeverPort\n");
        exit(1);
    }
    i =0;
    tmparg = strtok(tmppath,":");
    while(tmparg!=NULL)
    {
        strcpy(mypath[i],tmparg);
        tmparg = strtok(NULL,":");
        //printf("path[%d]=%s\n",i,mypath[i]);
        i++;
    }

    client_init(argv);

    printf("********  processing loop  *********\n");
    while (1)
    {
        printf("input a line : ");
        bzero(line, MAX);                // zero out line[ ]
        fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

        line[strlen(line)-1] = 0;        // kill \n at end
        if (line[0]==0)                  // exit if NULL line
            exit(0);

        // Send ENTIRE line to server

        if(line[0]=='l'&&line[1]!='s')
        {
            printf("local command!\n");
            execute(line, env);

        }// end localcmd
        else
        {
            n = write(sock, line, MAX);
            printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
            while (read(sock, ans, MAX)!=0)
            {
                if(strcmp(ans,"*`*END*`*")==0)
                    break;
                fwrite(ans,1,MAX,stdout);
                bzero(ans, MAX);
                n = write(sock,line,MAX);

            }
        }


    }
}


