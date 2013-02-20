#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main()
{
    struct stat s;
    char input[128];
    char *inputPtr = input;
    printf("input path: ");
    gets(inputPtr);
    if(input[0]=='\0')
    {
        strcpy(input,"/home/bryce/Downloads/EasyBCD 2.2.exe");
        lstat(input,&s);
    }
    else
        lstat(input,&s);

    if(S_ISLNK(s.st_mode))
    {
        //char[64] name =
        stat(input,&s);
        //printf("link %s\n",s.st_nlink);
    }

    printf("File: %s\n",input);
    printf("File_type\tPermissions\tUid\tSize\tCreation Time\n"
           "---------\t-----------\t---\t----\t-------------\n");

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
   // s.st_ctime;

    return 0;
}
