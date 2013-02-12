#include "header.h"



int main(int argc, char** argv, char **envp){
	int i=0;
	for(i=0; envp[i]!=0; i++){
		printf("envp[%d]: %s\n", i,envp[i]);
	}

	char command[64];
	char input[128];
	char *token;
	char *args;
	char temp[128];

	while(1){
		printf(ANSI_COLOR_GREEN "%s@sh " ANSI_COLOR_BLUE"%s $ " ANSI_COLOR_RESET,envp[9]+5,envp[17]+4);
		gets(input);
		strcpy(temp,input);
		token = strtok(temp, " \n");
		while(token!=NULL){
			token = strtok(NULL, " \n");
			printf("Token: %s\n",token);
			printf("TEmp: %s\n",temp);
			if(token!=NULL)
				strcpy(args,token);
		}
	}
	return 0;
}
