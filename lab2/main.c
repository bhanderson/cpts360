#include <stdio.h>
#include <stdlib.h>
#include <string.h>



typedef struct Node
{
	struct Node *parentPtr, *siblingPtr, *childPtr;
	char name[64];
	char type;
}Node;

void init(void);
Node *search(Node *current_node, char name[64]);
int insert(char name[64], char type, Node *parent);
int commandId(void);
int mkdir(char p[64]);
int makedir(char p[64]);
int ls(Node *leaf);

Node *root, *pwd;
char input[128], command[16], pathname[64],	dirname[64], basename[64];
//char *input, *command, *pathname, *dirname, *basename;
char *token, *context;

int main(){
	int id;
	init();

	while (1)
	{
		*input = "";
		*command = "";
		*pathname = "";
		if(commandline() == 1)
			return 0;
		id = commandId();
		switch(id){
			case 0: //mkdir
				makedir(pathname);
				break;
			case 1: //rmdir
				break;
			case 2: //cd
				break;
			case 3: //ls
				ls(root);
				break;
			case 4: //pwd
				break;
			case 5: //create
				break;
			case 6: //rm
				break;
			case 7: //save
				break;
			case 8: //reload
				break;
			case 9: //quit
				break;
			default:
				break;

		}
	}
	return 0;
}
int commandline(void){

	printf("Enter a command (q to quit): ");
	gets(input);
	if(strcmp(input,"q")==0||strcmp(input,"quit")==0) return 1;
	if(input[0] != '\0'){
		//*command = strtok(input, " ");
		strcpy(command,strtok(input, " "));
		if(input != NULL)
			strcpy(pathname,strtok(NULL, " "));
	}
	printf("Command: %s \n", command);
	printf("Pathname: %s \n", pathname);
	printf("input: %s \n", input);
	return 0;
}


void init(void){
	root = (Node *)malloc(sizeof(Node));
	root->parentPtr = 0;
	root->siblingPtr = 0;
	strcpy(root->name, "root");
	root->type = 'd';
	pwd = root;
}

int insert(char name[64], char type, Node *parent){
	Node *inserted = (Node *)malloc(sizeof(Node));
	inserted->type = type;
	strcpy(inserted->name, name);
	inserted->parentPtr = parent;
	inserted->childPtr = 0;
	inserted->siblingPtr = 0;
	return 0;
}


Node *search(Node *current_node, char name[64]){
	if(current_node->name != name && current_node->childPtr != 0){
		search(current_node->childPtr, name);
	}

	if(current_node->name != name && current_node->siblingPtr != 0){
		search(current_node->siblingPtr, name);
	}
	if(current_node->name == name){
		return current_node;
	}
	return 0;

}


int commandId(void){
	if(strcmp(command,"mkdir")==0)
		return 0;
	if(strcmp(command,"rmdir"))
		return 1;
	if(strcmp(command,"cd"))
		return 2;
	if(strcmp(command,"ls"))
		return 3;
	if(strcmp(command,"pwd"))
		return 4;
	if(strcmp(command,"create"))
		return 5;
	if(strcmp(command,"rm"))
		return 6;
	if(strcmp(command,"save"))
		return 7;
	if(strcmp(command,"reload"))
		return 8;
	if(strcmp(command,"quit")||strcmp(command,"q"))
		return 9;
	return -1;
}

int mkdir(char p[64]){
	Node *parent;
	char *parentName;
	if(strrchr(p,'/')!=NULL){// a / was found
		strcpy(dirname,strrchr(p,'/'));
		memmove(dirname + 0, dirname + 1, strlen(dirname));
		strcpy(pathname,p);
		pathname[strlen(p)-strlen(dirname)] = '\0';
		printf("Pathname: %s\n", pathname);
	}else{
		strcpy(dirname,p);
		insert(dirname, 'D', pwd);
	}

	return 0;
}

int makedir(char p[64]){
	char *path;
	Node *cwd = root;
	path = strtok(p,"/");
	while(path!=NULL){
		printf("Path: %s\n",path);
		path = strtok(NULL,"/");
		if(search(root, path)==0){//if not found insert at cwd
			insert(path, 'D', cwd);
		}else{// if found make cwd present directory
			cwd = search(root, path);
			insert(path, 'D', cwd);
		}
		cwd = search(root, path);
	}

	return 0;
}

int ls(Node *leaf){
	if(leaf!=0){
		printf(leaf->name);
		ls(leaf->childPtr);
		ls(leaf->siblingPtr);
	}
	return 0;
}
