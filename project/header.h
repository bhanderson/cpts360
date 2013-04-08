#ifndef HEADER_H
#define HEADER_H

#include "types.h"

PROC *running;
PROC *p0;
PROC *p1;

MINODE *root;
MINODE minode[100];

int fd;

char buff[BLOCK_SIZE];


void init()
{
	p0 = calloc(sizeof(PROC));
	p0->cwd = 0;
	p0->uid = 0;

	p1 = calloc(sizeof(PROC));
	p1->cwd = 0;
	p1->uid = 1;

	root = 0;

	sp = calloc(sizeof(ext2_super_block));
	gp = calloc(sizeof(ext2_group_desc));
	ip = calloc(sizeof(ext2_inode));

	for (i = 0; i < 100; i++) {
		minode[i].refCount = 0;
	}

	return;

}


#endif
