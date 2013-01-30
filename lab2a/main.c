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


int findCommand(char com[64]);
void init(void);
node *mkdir(char *n, char t, node *cwd);
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
            mkdir(strcat(basename,dirname), 'D',pwd);
            break;
        case 1:
            ls(pwd->childPtr);
            break;
        case 2:
			if(cd(basename,pwd->childPtr)!=-1) pwd = cd(basename,pwd->childPtr);
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
    root->type='D';
    strcpy(root->name,r);
    root->siblingPtr=0;
    root->childPtr=0;
	root->parentPtr=0;
    pwd = root;
}

node *mkdir(char *n, char t, node *cwd) {
	if(strchr(n,'/')==NULL){// if there is a NOT / in the input
		node *child;
		child = (node *)malloc(sizeof(node));
		strncpy(child->name,n,64);
		child->type='D';
		child->siblingPtr=pwd->childPtr;
		child->childPtr=0;
		child->parentPtr=pwd;
		cwd->childPtr = child;
		return pwd->childPtr;
	}

	// if there IS a / in the input

	char *token;
	node *temp;
	token = strtok(dirname,"/");
	printf("Token: %s",token);
	while(token!=NULL)
	{
		temp = cd(token, cwd);
		if(temp=-1)
		{
			cwd = mkdir(token, 'D',cwd);
			token = strtok(NULL,"/");
		}

		//token = strtok(dirname,"/");

	}
	return cwd;

}

node *cd(char *n,node *cwd) {
	if(*n==0)
	{
		cwd=root;
		return cwd;
	}

    while(cwd!=0) {
    	if(cwd->type!='D')
    	{
    		printf("Error not a directory!\n");
    		return pwd;
    	}
        if(strncmp(cwd->name,n,64)==0) {
            return cwd;
        } else {
            cwd = cwd->siblingPtr;
        }
    }
    //printf("Error directory not found\n");
    return -1;
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

void readinputplex()
{
	char *token, *dir;
	memset(&input[0],0,sizeof(input));
	memset(&pathname[0],0,sizeof(pathname));
	memset(&dirname[0],0,sizeof(dirname));
	memset(&basename[0],0,sizeof(basename));


	fgets(input, 128, stdin);
	token = strtok(input, " \n/");
	if(token!=NULL)
	{
		strcpy(command,token);
	}
	token = strtok(NULL, " \n");
	if(memchr(input,'/',128)!=NULL){// if there is a / in the input
		if(token!=NULL) dir = strrchr(token,'/')+1;
		if(dir!=NULL && token!=NULL)
		{
			strcpy(basename, dir);
		}
		while(token!=NULL) // TODO: change dirname to dirname - basename
		{
			strcpy(pathname,token);
			strncpy(dirname,pathname,strlen(pathname)-strlen(basename));
			token = strtok(NULL, "\n");
		}
		printf("Pathname: %s\n",pathname);
		printf("Dirname: %s\n",dirname);
		printf("Basename: %s\n",basename);

		return;
	}
	if(token!=NULL){
		strcpy(basename, token);
	}
	return;
}
