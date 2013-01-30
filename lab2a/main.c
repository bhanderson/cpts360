#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct node {
    struct node *siblingPtr, *childPtr, *parentPtr;
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
int rmdir(char n[64]);
void rm(node *child);
void readinputplex(void);

int main() {
    init();
    while(1) {
		printf("user@shell ");
		pwdir();
		printf(" $ ");

        readinputplex();
        switch (findCommand(command)) {
        case 0:
            mkdir(basename);
            break;
        case 1:
            ls(pwd->childPtr);
            break;
        case 2:
            cd(basename,pwd->childPtr);
            break;
        case 3:
            pwdir();
            break;
        case 4:
			rmdir(basename);
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
    char r[10] = "/";
    root = (node *)malloc(sizeof(node));
    root->type='d';
    strcpy(root->name,r);
    root->siblingPtr=0;
    root->childPtr=0;
	root->parentPtr=0;
    pwd = root;
}

node *mkdir(char *n) {
    node *child;
    child = (node *)malloc(sizeof(node));
    strncpy(child->name,n,64);
    child->type='d';
    child->siblingPtr=pwd->childPtr;
    child->childPtr=0;
	child->parentPtr=pwd;
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
        return;
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
    /*if(strchr(input,'/')!=NULL)
    {
    	readinputplex();
    	return;
    }*/
    if(token != NULL)
        strncpy(command, token, 64);
    while (token!=NULL) {
        //printf("%s ", token);
        token = strtok(NULL, " \n");
        if(token!=NULL)
            strncpy(basename, token, 64);

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
    if(strncmp(com,"rmdir",64)==0){
		return 4;
    }
    if(strncmp(com,"?",64)==0) {
        //printf("\t======================help============================\n\tmkdir ls q ?\n");
        return 8;
    }
    if(strncmp(com,"q",64)==0 ||strncmp(com,"quit",64)==0 ||strncmp(com,"exit",64)==0) {
        printf("QUIT\n");
        return 9;
    }

    printf("Error command not found type ? for help\n");
    return -1;

}

void pwdir() {
    node *cwd=pwd;
	char c[128] ="";
	if(pwd==root)strcat(c,"/");
	while(cwd !=root){

		strcat(c,cwd->name);
		strcat(c,"/");
		cwd = cwd->parentPtr;
	}

//	strrev(c);
	printf(c);
}

int rmdir(char n[64]){
	if(pwd->childPtr==0)
	{
		printf("Cannot find that directory\n");
		return -1;
	}
	node * cwd = pwd->childPtr;
	if(strncmp(cwd->name,n,64)==0){//if the first value is the rmdir
		pwd->childPtr=cwd->siblingPtr;
		free(cwd->childPtr);
		return 0;
	}
	while(cwd->siblingPtr!=0)			// check to see if no more siblings
	{
		if(strncmp(cwd->siblingPtr->name,n,64)==0)
		{
			node *temp;
			temp = cwd->siblingPtr->siblingPtr;
			free(cwd->siblingPtr);
			cwd->siblingPtr=temp;
			if(temp!=0)
				rm(temp->childPtr);
			free(temp);
			return 0;
		}
		cwd=cwd->siblingPtr;
	}
	printf("Cannot find that directory\n");
	return -1;
}

void rm(node *child){
	if(child!=0){
		rm(child->siblingPtr);
		rm(child->childPtr);
		free(child);
	}
}
void readinputplex(void)
{
	char *token;
    fgets(input, 128, stdin);
    if(strchr(input,'/')!=NULL)//if path extract basename dirname and pathname
    {
    	char *cmd;
		int pos;
		cmd=strchr(input,' ');

		pos=cmd-input;
		if(pos<0)
		{
			printf("invalid command");
			return -1;
		}
		strncpy(command, input,pos);


    	char *laslash = strrchr(input, '/');
		int b = strlen(input)-(laslash-input);
		char *temp;
		strncpy(basename,laslash+1,b-2); // basename done



    	return;
    }

    token = strtok(input," \n");
    if(token != NULL)
        strncpy(command, token, 64);
    while (token!=NULL) {
        token = strtok(NULL, " \n");
        if(token!=NULL)
            strncpy(basename, token, 64);

    }
    return 0;




}
