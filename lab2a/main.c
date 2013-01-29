#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct node {
    struct node *siblingPtr, *childPtr;
    char type;
    char name[64];
} node;

node *root, *pwd;
char input[128], command[64], pathname[64], dirname[64], basename[64];
int readinput(void);
int findCommand(char com[64]);
void init(void);
node *mkdir(char *n);
node *cd(char *n, node *cwd);
void ls(node *cwd);
void pwdir(void);

int main() {
    init();
    while(1) {
        readinput();
        switch (findCommand(command)) {
        case 0:
            mkdir(dirname);
            break;
        case 1:
            ls(pwd->childPtr);
            break;
        case 2:
            cd(dirname,pwd->childPtr);
            break;
        case 3:
            pwdir();
            break;
        case 4:
            break;
        case 9:
            return 0;
            break;

        default:
            break;
        }

    }

    return EXIT_SUCCESS;
}

void init(void) {
    char r[10] = "root";
    root = (node *)malloc(sizeof(node));
    root->type='d';
    strcpy(root->name,r);
    root->siblingPtr=0;
    root->childPtr=0;
    pwd = root;
}

node *mkdir(char *n) {
    node *child;
    child = (node *)malloc(sizeof(node));
    strncpy(child->name,n,64);
    child->type='d';
    child->siblingPtr=pwd->childPtr;
    child->childPtr=0;
    pwd->childPtr = child;
    return pwd->childPtr;
}

node *cd(char *n,node *cwd) {
    while(cwd!=0) {
        if(strncmp(cwd->name,n,64)==0) {
            pwd = cwd;
            return cwd;
        } else {
            cwd = cwd->siblingPtr;
        }
    }
    printf("Error directory not found\n");
    return 0;
}

void ls(node *cwd) {
    if(cwd==0)
        printf("nothing to be found here\n");
    while(cwd != 0) {
        printf("%s ", cwd->name);
        cwd = cwd->siblingPtr;
    }
    printf("\n");


}

int readinput() {
    char *token;

    fgets(input, 128, stdin);
    token = strtok(input," \n");
    if(token != NULL)
        strncpy(command, token, 64);
    while (token!=NULL) {
        //printf("%s ", token);
        token = strtok(NULL, " \n");
        if(token!=NULL)
            strncpy(dirname, token, 64);

    }
    return 0;
}

int findCommand(char com[64]) {
    if(!strncmp(com,"mkdir",64)) {
        //printf("MAKEDRIVE\n");
        return 0;
    }
    if(strncmp(com,"ls",64)==0) {
        //printf("LS\n");
        return 1;
    }
    if(strncmp(com,"cd",64)==0) {
        //printf("CD\n");
        return 2;
    }
    if(strncmp(com,"pwd",64)==0) {
        //printf("PWD\n");
        return 3;
    }
    if(strncmp(com,"?",64)==0) {
        //printf("\t======================help============================\n\tmkdir ls q ?\n");
        return 8;
    }
    if(strncmp(com,"q",64)==0) {
        printf("QUIT\n");
        return 9;
    }

    printf("Error command not found type ? for help\n");
    return -1;

}

void pwdir() {
    printf("%s\n",pwd->name);
}



