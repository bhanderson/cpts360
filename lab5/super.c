#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ext2fs/ext2_fs.h>
#define block_size 1024
int main(int argc, char *envp []){
	int fd;
	char buff[1024];
	struct ext2_super_block super_block;
	struct ext2_group_desc gd_block;

	fd = open("/home/bryce/Downloads/fdimage", O_RDONLY);

	read(fd, buff, 1024);
	read(fd, &super_block, sizeof(struct ext2_super_block));
	printf("********** super block info: **********\n");
	printf("inode_count\t\t%d\n", super_block.s_inodes_count);
	printf("blocks_count\t\t%d\n", super_block.s_blocks_count);
	printf("r_blocks_count\t\t%d\n", super_block.s_r_blocks_count);
	printf("free_blocks_count\t%d\n", super_block.s_free_blocks_count);
	printf("free_inodes_count\t%d\n", super_block.s_free_inodes_count);
	printf("log_blk_size\t\t%d\n", super_block.s_log_block_size);
	printf("first_data_block\t%d\n", super_block.s_first_data_block);
	printf("magic number\t\t0X%x\n", super_block.s_magic);
	printf("rev_level\t\t%d\n", super_block.s_rev_level);
	printf("inode_size\t\t%d\n",super_block.s_inode_size);
	printf("block_group_nr\t\t%d\n",super_block.s_block_group_nr);
	printf("blksize\t\t\t%d\n",block_size);
	printf("inode_per_group\t\t%d\n", super_block.s_inodes_per_group);
	printf("----------------------------------------\n");
	printf("************* group 0 info ************");


	lseek(fd, block_size*3 ,SEEK_CUR);
	read(fd, &gd_block, sizeof(struct ext2_group_desc));



	printf("block bitmap:\t%d\n", gd_block.bg_block_bitmap);
	printf("free blocks:\t%d\n", gd_block.bg_free_blocks_count);
	printf("used dirs:\t%d\n", gd_block.bg_used_dirs_count);



	close(fd);

    return 0;
}
