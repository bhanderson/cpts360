#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ext2fs/ext2_fs.h>
#define block_size 1024
int main(int argc, char *envp []){
	int fd;
	char buff[1024];
	struct ext2_group_desc gd_block;

	fd = open("/home/bryce/Downloads/fdimage", O_RDONLY);

	lseek(fd, block_size*2,SEEK_CUR);
	read(fd, &gd_block, sizeof(struct ext2_group_desc));

	printf("block bitmap:\t%d\n", gd_block.bg_block_bitmap);
	printf("free blocks:\t%d\n", gd_block.bg_free_blocks_count);
	printf("used dirs:\t%d\n", gd_block.bg_used_dirs_count);

	close(fd);
}
