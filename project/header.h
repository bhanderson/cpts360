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
int findCmd(char *cname);
int make_dir();
int my_mkdir(MINODE *pip, char *name);

PROC *running;
PROC *p0;
PROC *p1;

MINODE *root;
MINODE minode[100];

int fd;

char buff[BLOCK_SIZE];


void init(){ /*{{{*/
	int i;
	p0 = calloc(1, sizeof(PROC));
	p0->cwd = 0;
	p0->uid = 0;

	p1 = calloc(1, sizeof(PROC));
	p1->cwd = 0;
	p1->uid = 1;

	root = 0;

	sp = calloc(1, sizeof(SUPER));
	gp = calloc(1, sizeof(GD));
	ip = calloc(1, sizeof(INODE));

	for (i = 0; i < 100; i++) {
		minode[i].refCount = 0;
	}

	return;
} /*}}}*/

void mount_root( char *path ){ /*{{{*/
	fd = open(path, O_RDWR );
	lseek(fd, BLOCK_SIZE, SEEK_SET );
	read(fd, sp, sizeof(SUPER));
	lseek(fd, BLOCK_SIZE*GDBLOCK, SEEK_SET);
	read(fd, gp, sizeof(GD));

	if(sp->s_magic != SUPER_MAGIC){
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

void print_block(int dev, int block){ /*{{{*/
	int i,j;
	char arr[1024];
	lseek(dev, block*BLOCK_SIZE, SEEK_SET);
	for (i = 0; i < 1024; i++) {
		for (j = 0; j < 8; j++) {
			printf("%b ", arr[i] & 1 << j);
		}
	}
} /*}}}*/

int findCmd(char *cname){ /*{{{*/
	if(strcmp(cname, "mkdir")==0)
		return 0;
	if(strcmp(cname, "cd")==0)
		return 1;
	if(strcmp(cname, "pwd")==0)
		return 2;
	if(strcmp(cname, "ls")==0)
		return 3;
	if(strcmp(cname, "mount")==0)
		return 4;
	if(strcmp(cname, "umount")==0)
		return 5;
	if(strcmp(cname, "creat")==0)
		return 6;
	if(strcmp(cname, "rmdir")==0)
		return 7;
	if(strcmp(cname, "rm")==0)
		return 8;
	if(strcmp(cname, "open")==0)
		return 9;
	if(strcmp(cname, "close")==0)
		return 10;
	if(strcmp(cname, "read")==0)
		return 11;
	if(strcmp(cname, "write")==0)
		return 12;
	if(strcmp(cname, "cat")==0)
		return 13;
	if(strcmp(cname, "cp")==0)
		return 14;
	if(strcmp(cname, "mv")==0)
		return 15;
	if(strcmp(cname, "pfd")==0)
		return 16;
	if(strcmp(cname, "lseek")==0)
		return 17;
	if(strcmp(cname, "rewind")==0)
		return 18;
	if(strcmp(cname, "stat")==0)
		return 19;
	if(strcmp(cname, "pm")==0)
		return 20;
	if(strcmp(cname, "menu")==0)
		return 21;
	if(strcmp(cname, "access")==0)
		return 22;
	if(strcmp(cname, "chmod")==0)
		return 23;
	if(strcmp(cname, "chown")==0)
		return 23;
	if(strcmp(cname, "cs")==0)
		return 23;
	if(strcmp(cname, "fork")==0)
		return 23;
	if(strcmp(cname, "ps")==0)
		return 23;
	if(strcmp(cname, "kill")==0)
		return 23;
	if(strcmp(cname, "quit")==0)
		return 23;
	if(strcmp(cname, "touch")==0)
		return 23;
	if(strcmp(cname, "sync")==0)
		return 23;
	if(strcmp(cname, "link")==0)
		return 23;
	if(strcmp(cname, "unlink")==0)
		return 23;
	if(strcmp(cname, "symlink")==0)
		return 23;
	if(strcmp(cname, "exit")==0)
		return 99;
	return -1;
} /*}}}*/

int make_dir(){ /*{{{*/
	char line[128], parent[64], child[64];
	int dev;

	printf("enter the pathname ");
	fgets(line, 128, stdin);
	line[strlen(line)-1]=0;
	if(line[0] == '/')
		dev = root->dev;
	else
		dev = running->cwd->dev;
	
	strncpy(child, strrchr(line, '/')+1, 64);
	printf("child: %s\n", child);

	strncpy(parent, line, strcspn(line,child)-1);
	parent[strcspn(line,child)-1]=0;
	printf("parent: %s\n", parent);
	return 1;

} /*}}}*/




int my_mkdir(MINODE *pip, char *name);



#endif
