#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

void printstat(struct stat s);
void getinput();
//void clearstat(struct stat s);

char input[128];

int main()
{
    struct stat s;
    char *inputPtr=input;
    getinput(inputPtr);
    //check if exiting or invalid file
    while(strcmp(inputPtr,"exit")!=0 && lstat(input,&s)==0)
    {
        if(S_ISLNK(s.st_mode))   // if s is a link print filename -> linkname
        {
            char *filename;     // input filename
            filename = strrchr(input,'/');
            char name[64] = {0};
            char *namePtr;      // linked filename
            readlink(input, name, 64);
            namePtr = strrchr(name,'/');
            stat(input,&s);
            printstat(s);
            printf(" %s -> %s\n",filename+1,namePtr+1);// +1 gets rid of /
            printf("\n");

        }
        else if(S_ISDIR(s.st_mode))     // if s is a dir
        {
            DIR *dp = opendir(input);   // open the directory
            struct dirent *ep = readdir(dp);    // browse to first file
            // print header
            printf("File_type\tPermissions\tUid\tSize\tCreation Time\n"
                   "---------\t-----------\t---\t----\t-------------\n");
            // while there are more files in directory
            while((ep = readdir(dp))!=0)
            {
                char c[128];
                strncat(c,input,64);
                strcat(c,"/");
                strncat(c,ep->d_name,64); // create path
                lstat(c,&s);
                printstat(s);
                printf(" %s",ep->d_name);
                printf("\n");
                //printf("C: %s \n",c);
                //clearstat(s);
                c[0]='\0';

            }
            //printf("\n");

        }
        else
        {

            printf("File_type\tPermissions\tUid\tSize\tCreation Time\n"
                   "---------\t-----------\t---\t----\t-------------\n");
            printstat(s);
            printf(" %s\n",strrchr(input,'/')+1);
            printf("\n");
        }


        getinput();

    }// end program loop
    printf("Invalid file or exit string\n");
    return 0;
}

void printstat(struct stat s)
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

    char c[64];
    char *i = c;
    ctime_r(&s.st_ctime,i);
    i[strlen(i)-1]='\0';
    printf("%s",i);
}

void getinput()   // get an input from the user type: /home/bryce/...
{
    char in[128];
    printf("input path: ");
    gets(in);
    strcpy(input,in);
}


