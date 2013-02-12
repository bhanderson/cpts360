#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char **argv)
{
    int i = 0;
    while(i<argc){
        printf("argv[%d}: %s \n",i,argv[i]);
        i++;
    }
    printf("PWD: %s\n",get_current_dir_name());
    chdir("/etc/");
    printf("PWD: %s\n",get_current_dir_name());

    return 0;
}
