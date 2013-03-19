#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ext2fs/ext2_fs.h>
#define block_size 1024
int main(int argc, char *envp []){
	int fd, i, j;
	char buff[block_size];
	char out[block_size];
	fd = open("/home/bryce/Downloads/fdimage", O_RDONLY);

	lseek(fd, block_size*3,SEEK_CUR);
	read(fd, buff, block_size);
	for(j=0;j<block_size;j++){
		for(i=0;i<8;i++){
		printf("%d",buff[j] >> i & 1);
		}
		printf(" ");
		if(j%10==0 && j!=0||j==10)
			printf("\n");
	}
	printf("\n");
	close(fd);

	return 0;
}

