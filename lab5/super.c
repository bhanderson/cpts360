#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ext2fs/ext2_fs.h>
#define BLOCK_SIZE 1024

int main(int argc, char *argv [])
{
    typedef struct ext2_super_block super_block;
    typedef struct ext2_group_desc gd_block;
    typedef struct ext2_inode i_block;
    typedef struct ext2_dir_entry_2 dir;

	super_block *sp;
	gd_block	*gp;
	i_block		*ip;
	dir			*dp;

    int fd, i, j;
    unsigned char buff[BLOCK_SIZE];
	printf("%s %d\n",argv[1], argc);
    if(argc < 2){
		printf("Usage: VDISK\n");
		exit(1);
    }
	fd = open(argv[1], O_RDONLY);
	//printf("")

    if (fd < 0){
		printf("open failed\n");
		exit(1);
    }

    lseek(fd, BLOCK_SIZE, SEEK_SET);
    read(fd, buff, BLOCK_SIZE); // read super block
    sp = (super_block *)buff;
    printf("********** super block info: **********\n");
    printf("inode_count\t\t%d\n", sp->s_inodes_count);
    printf("blocks_count\t\t%d\n", sp->s_blocks_count);
    printf("r_blocks_count\t\t%d\n", sp->s_r_blocks_count);
    printf("free_blocks_count\t%d\n", sp->s_free_blocks_count);
    printf("free_inodes_count\t%d\n", sp->s_free_inodes_count);
    printf("log_blk_size\t\t%d\n", sp->s_log_block_size);
    printf("first_data_block\t%d\n", sp->s_first_data_block);
    printf("magic number\t\t0x%X\n", sp->s_magic);
    printf("rev_level\t\t%d\n", sp->s_rev_level);
    printf("inode_size\t\t%d\n",sp->s_inode_size);
    printf("block_group_nr\t\t%d\n",sp->s_block_group_nr);
    printf("blksize\t\t\t%d\n", sp->s_log_block_size + 1 * 1024);
    printf("inode_per_group\t\t%d\n", sp->s_inodes_per_group);

    getchar();


    printf("************* group 0 info ************\n");
    read(fd, buff, BLOCK_SIZE); // gd block
    gp = (gd_block *)buff;

    printf("block bitmap:\t\t%d\n", gp->bg_block_bitmap);
    printf("Inodes bitmap block:\t%d\n", gp->bg_inode_bitmap);
    printf("Inodes table block:\t%d\n", gp->bg_inode_table);
    printf("Free blocks count:\t%d\n", gp->bg_free_blocks_count);
    printf("Free inodes count:\t%d\n", gp->bg_free_inodes_count);
    printf("Directories count:\t%d\n", gp->bg_used_dirs_count);
    printf("inodes_start\t\t%d\n", gp->bg_inode_table);
    getchar();
	int inode_block = gp->bg_inode_table;

    printf("*********** BMAP ***********\n");
    lseek(fd, BLOCK_SIZE*3, SEEK_SET);
    read(fd, buff, BLOCK_SIZE); // read bmap block

    for(i=0; i<24; i++)
    {
        if(i%8==0)
            printf("\n");
		else
			printf(" ");
        for(j=0; j<8; j++)
        {
            printf("%d", buff[i] >> j & 1);
        }
    }
    getchar();


    printf("\n*********** IMAP ***********");
    lseek(fd, BLOCK_SIZE*4, SEEK_SET);

    read(fd, buff, BLOCK_SIZE); // read imap

    for(i=0; i<24; i++)
    {
        if(i%8==0)
            printf("\n");
		else
			printf(" ");
        for(j=0; j<8; j++)
        {
            printf("%u", buff[i] >> j & 1);
        }
    }
    printf("\n");
	getchar();

    printf("\n*********** INODE ***********\n");
	lseek(fd, BLOCK_SIZE*inode_block+128, SEEK_SET);
    read(fd, buff, BLOCK_SIZE);

	ip = (i_block *)buff;
	printf("inode_block= %d\n", inode_block);
	printf("inode_mode = %x\n", ip->i_mode);
	printf("uid: %d\n", ip->i_uid);
	printf("size: %d\n",ip->i_size);
	printf("ctime: %s", ctime(&ip->i_ctime));
	printf("gid: %d\n",ip->i_gid);
	printf("links: %d\n",ip->i_links_count);
	printf("blocks: %d\n",ip->i_blocks);
	printf("i_block[0]=%d\n",ip->i_block[0]);
	printf("\n");
	getchar();

	printf("*********** DIR ***********\n");
	int data_block = ip->i_block[0];

	lseek(fd, BLOCK_SIZE*data_block, SEEK_SET);
	read(fd, buff, BLOCK_SIZE);
	dp = (dir *)buff;
	char *cp = buff;

	while(cp < buff + 1024){
		char name[dp->name_len];
		memcpy(name, dp->name, dp->name_len+1);
		name[dp->name_len]='\0';

		printf("%d\t%d\t%d\t%s\n",dp->inode,dp->rec_len, dp->name_len, name);
		cp += dp->rec_len;
		dp = (dir *)cp;
	}





    close(fd);

    return 0;
}

