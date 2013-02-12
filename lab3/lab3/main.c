#include "header.h"
char command[64], input[128], str[128];
int i = 0;



int main(int argc, char** argv, char **envp)
{
    for(i=0; envp[i]!=0; i++)
    {
        printf("envp[%d]: %s\n", i,envp[i]);
    }
    while(1)
    {
        printf(ANSI_COLOR_GREEN "%s@MySh " ANSI_COLOR_BLUE"%s $ " ANSI_COLOR_RESET,envp[2]+5,get_current_dir_name());
        fgets(input,128,stdin);
        strcpy(str,input);
        //printf("input: %s\n",str);
        char *token = strtok(str, " \n");

        if(token!=NULL)
            strcpy(command, token);
        //printf("Command: %s\n",command);
        if(strncmp(command, "exit",64)==0||strncmp(command, "q",64)==0)
            exit(1);

        char args[64][64] = {0};
        i=0;
        while(token!=NULL)
        {
            token = strtok(NULL, " \n");
            if(token!=NULL)
            {
                strcpy(args[i],token);
                printf("Args[%d]: %s\n",i,args[i]);
                i++;
            }
            //token = strtok(NULL, " \n");
        }
        //printf("Args[0]: %x\n",args[0]);
        //printf("command: %s\n",command);
        if(strncmp(command, "cd",64)==0)
        {
            //printf("Changing directory soon\n");
            if(strncmp(args[0],"\0",64)==0)
            {
                //printf("CWD:%s\n",get_current_dir_name());
                chdir(envp[5]+5);
                printf("CWD:%s\n",get_current_dir_name());
            }
            else   // end if cd NULL
            {
                //printf("CWD:%s\n",get_current_dir_name());
                if(chdir(args[0])==-1)
                    printf("chdir error\n");
                printf("CWD:%s\n",get_current_dir_name());
            }//end else
        }
        else   // end if(cd)
        {
            pid_t child_pid, wpid;
            int status = 0, exe;
            printf("parent_pid = %d\n", getpid());
            if((child_pid = fork()) == 0)
            {
                printf("In child process (pid = %d)\n", getpid());
                char cat[64];
                strcat(cat, "/bin/");
                strcat(cat, command);
                printf("cat: %s command: %s\n",cat,command);
                int exe = execve(cat,argv,envp);
                exit(2);
            }
            while((wpid = wait(&status)) > 0)
            {
                printf("Exit status of %d was %d\n", (int)wpid, status);
                printf("EXE: %d\n",exe);
            }

        }// end else

    }
    return 0;
}
