#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ext2fs/ext2_fs.h>
#define block_size 1024

int main(int argc, const char *argv[])
{
	char buff[1024];
	int fd, i, j;
	fd = open("diskimage", O_RDONLY);
	printf("char size %d\n", sizeof(char));
	while(read(fd, buff, 1024)!=0){
		for(i=0;i<1024;i++){
			//for (j = 0; j < 8; j++) {
				printf("%X ",buff[i]);// >> j & 1);
			//}
		}
		printf("\n");
	}
	return 0;
}
