#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>

char command[64], input[128], str[128];
int i = 0;

int main(int argc, char** argv, char **envp)
{
    /*
    for(i=0; envp[i]!=0; i++)
    {
        printf("envp[%d]: %s\n", i,envp[i]);
    }
    */
    while(1)
    {
        printf("\x1b[32m" "%s@MySh " "\x1b[34m" "%s $ " "\x1b[0m",envp[2]+5,get_current_dir_name());
        fgets(input,128,stdin);
        strcpy(str,input);

        char *token = strtok(str, " \n");
        if(token!=NULL)
        {
            strcpy(command, token);
        }

        if(strncmp(command, "exit",64)==0||strncmp(command, "q",64)==0)
        {
            exit(1);
        }

        char args[64][64];
        char **myargv=args;
        i=0;
        while(token!=NULL)
        {
            myargv[i] = token;
            i++;
            token = strtok(NULL, " \n");
        }
        myargv[i]=NULL;

        for(i =0; myargv[i]!=0; i++)
        {
            printf("myargv[%d]: %s\n",i,myargv[i]);
        }
        i++;
        printf("myargv[%d]: %s\n",i,myargv[i]);

        if(strncmp(command, "cd",64)==0)
        {
            if(strncmp(myargv[1],"\0",64)==0)
            {
                chdir(envp[5]+5);
                printf("CWD:%s\n",get_current_dir_name());
            }
            else
            {
                if(chdir(myargv[1])==-1)
                    printf("chdir error\n");
                printf("CWD:%s\n",get_current_dir_name());
            }
        }
        else
        {
            pid_t pid;
            int status;
            char *envp[] = { NULL };
            char temp[64] = "/bin/";
            myargv[0] = strcat(temp,command);
            printf("myargv[0]=%s\n",myargv[0]);

            switch (pid = fork())
            {
            case -1:
                perror("fork()");
                exit(EXIT_FAILURE);
            case 0:
                status = execve(myargv[0],myargv,envp);
                //break;
                exit(status);
            default:
                if( waitpid(pid, &status, 0) < 0)
                {
                    perror("waitpid()");
                    exit(EXIT_FAILURE);
                }
                if( WIFEXITED(status))
                {
                    break; // do not exit get next input
                    //exit(WEXITSTATUS(status));
                }
                exit(EXIT_FAILURE);
            }


        }// end else

    }
    return 0;
}
