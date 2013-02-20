#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

void print(struct stat s);
void getinput();

char input[128];

int main()
{
    struct stat s;
    char *inputPtr=input;
    getinput(inputPtr);
    //printf("input: %s",input);
    while(strcmp(inputPtr,"exit")!=0 && lstat(input,&s)==0)
    {

        if(S_ISLNK(s.st_mode)) // if s is a link print filename -> linkname
        {
            char *filename;
            filename = strrchr(input,'/');

            char name[64];
            char *namePtr;
            readlink(input, name, 64);
            namePtr = strrchr(name,'/');
            printf("File: %s -> %s\n",filename+1,namePtr+1);
            stat(input,&s);
        }
        else if(S_ISDIR(s.st_mode)) // if s is a dir
        {
            DIR *dp = opendir(input);
            struct dirent *ep = readdir(dp);
            while((ep = readdir(dp))!=0)
            {
                stat(ep->d_name,&s);
                printf("%s ",ep->d_name);
            }
            printf("\n");

        }
        else
        {
            printf("File: %s\n",strrchr(input,'/')+1);
            printf("File_type\tPermissions\tUid\tSize\tCreation Time\n"
                   "---------\t-----------\t---\t----\t-------------\n");
            print(s);
        }


        getinput();

    }// end program loop
    printf("Invalid filename\n");
    return 0;
}

void print(struct stat s)
{
    if(S_ISREG(s.st_mode)) printf("REG\t\t");
    if(S_ISDIR(s.st_mode)) printf("DIR\t\t");
    if(S_ISLNK(s.st_mode)) printf("LNK\t\t");
    //user permissions
    printf((s.st_mode & 1 << 8) ? "r" : "-");
    printf((s.st_mode & 1 << 7) ? "w" : "-");
    printf((s.st_mode & 1 << 6) ? "x" : "-");
    //group permissions
    printf((s.st_mode & 1 << 5) ? "r" : "-");
    printf((s.st_mode & 1 << 4) ? "w" : "-");
    printf((s.st_mode & 1 << 3) ? "x" : "-");
    //other permissions
    printf((s.st_mode & 1 << 2) ? "r" : "-");
    printf((s.st_mode & 1 << 1) ? "w" : "-");
    printf((s.st_mode & 1 << 0) ? "x" : "-");
    printf("\t");//space to uid
    printf("%d\t",s.st_uid);
    printf("%d\t",s.st_size);
    printf("%s\t",ctime(&s.st_ctime));
    printf("\n");
}

void getinput()
{
    char in[128];
    printf("input path: ");
    gets(in);
    strcpy(input,in);
}


