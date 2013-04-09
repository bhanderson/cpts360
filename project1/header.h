
#ifndef HEADER_H
#define HEADER_H

#include "types.h"

void init();
void mount_root(char *path);
MINODE *iget(int dev, unsigned long ino);
void iput(MINODE *mip);
void get_block(int dev, int block, char *buf);
void put_block(int dev, int block, char *buf);
unsigned long getino(int *dev, char *pathname);
void print_block(int dev, int block);

PROC *running;
PROC *p0;
PROC *p1;

MINODE *root;
MINODE minode[100];

int fd;

char buff[BLOCK_SIZE];


void init(){ /*{{{*/
	int i;
	p0 = malloc(sizeof(PROC));
	p0->cwd = 0;
	p0->uid = 0;

	p1 = malloc(sizeof(PROC));
	p1->cwd = 0;
	p1->uid = 1;

	root = 0;

	sp = malloc(sizeof(SUPERBLOCK));
	gp = malloc(sizeof(GDBLOCK));
	ip = malloc(sizeof(INODE));

	for (i = 0; i < 100; i++) {
		minode[i].refCount = 0;
	}

	return;
} /*}}}*/

void mount_root( char *path ){ /*{{{*/
	fd = open(path, O_RDWR );
	lseek( fd, BLOCK_SIZE, SEEK_SET );
	read( fd, sp, sizeof(SUPER));

	if(sp->s_magic != 0xEF53){
		printf("%x\n", sp->s_magic);
		printf("%d\n",fd);
		perror("NOT AN EXT2FS EXITING");
		exit(-1);
	}
	root = iget(fd, 2);
	p1->cwd = root;
	root->refCount = 2;
	running = p1;
} /*}}}*/

MINODE *iget(int dev, unsigned long ino){ /*{{{*/
	int i, freeNode, block, pos;
	for (i = 0; i < 100; i++) {
		if(minode[i].ino == 0){
			if(freeNode == 0)
				freeNode = i;
		}
		if(minode[i].ino == ino){
			minode[i].refCount ++;
			return &minode[i];
		}
	}

	// else it is not in the minode array so find it on the disk
	block = (ino-1) / INODES_PER_BLOCK + 5; // TODO + inode_table but 5 works
	pos = (ino-1) % INODES_PER_BLOCK;
	get_block(fd, block, (char *)&buff);

	minode[i].dev = dev;
	minode[i].dirty = 0;
	minode[i].INODE = *( ((INODE*)buff) + pos); // david barajas c magic
	minode[i].ino = ino;
	minode[i].mounted = 0;
	minode[i].mountptr = NULL;
	minode[i].refCount = 1;

	return &minode[i];
} /*}}}*/

void iput(MINODE *mip){ /*{{{*/
	int block, pos;
	mip->refCount--;
	if(mip->refCount > 0)
		return;
	if(mip->dirty == 0)
		return;
	// else
	block = (mip->ino - 1) / INODES_PER_BLOCK + 5;
	pos = (mip->ino - 1) % INODES_PER_BLOCK;
	get_block(fd, block, (char *)&buff);
	lseek((int)&buff, pos, SEEK_SET);
	write((int)&buff, mip, sizeof(INODE));
	put_block(fd, block, buff);
	return;
} /*}}}*/

void get_block(int dev, int block, char *buf){ /*{{{*/
	lseek(dev,(long)BLOCK_SIZE*block,SEEK_SET);
	read(dev, buf, BLOCK_SIZE);
} /*}}}*/

void put_block(int dev, int block, char *buf){ /*{{{*/
	lseek(dev, (long)BLOCK_SIZE*block, SEEK_SET);
	write(dev, buf, BLOCK_SIZE);
	return;
} /*}}}*/

unsigned long getino(int *dev, char *pathname){ /*{{{*/

	return 0;
} /*}}}*/

void print_block(int dev, int block){
	int i,j;
	char arr[1024];
	lseek(dev, block*BLOCK_SIZE, SEEK_SET);
	for (i = 0; i < 1024; i++) {
		for (j = 0; j < 8; j++) {
			printf("%b ", arr[i] & 1 << j);
		}
	}
}
#endif
