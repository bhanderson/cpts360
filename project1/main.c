#include "header.h"

int main(int argc, const char *argv[])
{
	char line[128], command[32], path[64], filepath[64];
	int cmd;

	init();

	printf("input diskimage: ");
	fgets(line, 128, stdin);
	line[strlen(line)-1] = 0; 		// null terminate line
	mount_root(line);

	while(1){
		printf("input command: ");


	}
	return 0;
}
