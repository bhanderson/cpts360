#ifndef HEADER_H
#define HEADER_H

#include "types.h"

void init();
void mount_root(char *path);
MINODE *iget(int dev, unsigned long ino);
void iput(MINODE *mip);
void get_block(int dev, unsigned long block, char *buf);
void put_block(int dev, int block, char *buf);
unsigned long getino(int *dev, char *pathname);
void print_block(int dev, int block);
int findCmd(char *cname);
int make_dir();
int my_mkdir(MINODE *pip, char *name);
unsigned long search(INODE *inodePtr, char *name);
unsigned long ialloc(int dev);
void idealloc(int dev, unsigned long ino);
unsigned long balloc(int dev);
void bdealloc(int dev, unsigned long ino);
void ls(char *pathname, PROC *parent);
void printdir(INODE *inodePtr);
void cd(char *pathname);

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

/* takes in a diskname path and opens it, stores superblock and group descriptor
 * block is sp and gp respectively, also check to see if file is ext2 and exits
 * if it is not sets root to root of fd and sets running proc
 */
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

/* this function looks for the inode in the minode array and returns.  if it
 * cannot find the minode it loads it from dev and returns the location in
 * minode array.  
 */
MINODE *iget(int dev, unsigned long ino){ /*{{{*/
	int i=0, freeNode=0, block=0, pos=0;
	freeNode=0;
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
	block = (ino-1) / INODES_PER_BLOCK + gp->bg_inode_table; // TODO + inode_table but 5 works
	pos = (ino-1) % INODES_PER_BLOCK;
	get_block(fd, block, (char *)&buff);

	minode[freeNode].dev = dev;
	minode[freeNode].dirty = 0;
	minode[freeNode].INODE = *( ((INODE*)buff) + pos); // david barajas c magic
	minode[freeNode].ino = ino;
	minode[freeNode].mounted = 0;
	minode[freeNode].mountptr = NULL;
	minode[freeNode].refCount = 1;

	return &minode[freeNode];
} /*}}}*/

/* takes a minode * and writes it to fd
 */
void iput(MINODE *mip){ /*{{{*/
	int block, pos;
	mip->refCount--;
	INODE *itmp;
	//	if(mip->refCount > 0)
	//		return;
	//	if(mip->dirty == 0)
	//		return;
	// else
	block = (mip->ino - 1) / INODES_PER_BLOCK + gp->bg_inode_table;
	pos = (mip->ino - 1) % INODES_PER_BLOCK;

	itmp = ( (INODE *)buff + pos);
	get_block(fd, block, (char *)&buff);
	memcpy( itmp, &mip->INODE, 128 );
	

//	lseek((int)&buff, pos, SEEK_SET);
//	write((int)&buff, &mip->INODE, sizeof(INODE));
	put_block(fd, block, buff);
	return;
} /*}}}*/

/* gets a block of data from dev and stores it to buf
 */
void get_block(int dev, unsigned long block, char *buf){ /*{{{*/
	lseek(dev,(long)BLOCK_SIZE*block,SEEK_SET);
	read(dev, buf, BLOCK_SIZE);
} /*}}}*/

/* puts a block of data from buf and stores it to fd
 */
void put_block(int dev, int block, char *buf){ /*{{{*/
	lseek(dev, BLOCK_SIZE*block, SEEK_SET);
	write(dev, buf, BLOCK_SIZE);
	return;
} /*}}}*/

/* traverses the pathname to the last point in pathaname and returns the inode
 * number of that inode 
 * TODO error checking, and cases
 */
unsigned long getino(int *dev, char *pathname){ /*{{{*/
	INODE *cwd = malloc(sizeof(INODE));
	char path[128], *token;
	int inodeIndex, seek;

	strncpy(path, pathname, 128);

	if(pathname[0] == '/'){
		strncpy(path, path+1, 127); // remove root symbol
		token = strtok(path, "/");
		cwd = &root->INODE;

		while(token !=NULL){
			inodeIndex = search(cwd, token);
			seek = ((inodeIndex-1) / 8 + gp->bg_inode_table)*BLOCK_SIZE +
				(inodeIndex-1)%8 * 128;
			lseek(fd, seek, SEEK_SET);

			read(fd, cwd, sizeof(INODE));
			token = strtok(NULL, "/");
		}
		return inodeIndex;
	}else {
		token = strtok(path, "/");
		cwd = &root->INODE;

		while(token != NULL) {
			inodeIndex = search(cwd, token);
			seek = ((inodeIndex-1) / 8 + gp->bg_inode_table)*BLOCK_SIZE +
				(inodeIndex-1)%8 * 128;
			lseek(*dev, seek, SEEK_SET);
			read(fd, cwd, sizeof(INODE));
			token = strtok(NULL, "/");
		}
		return inodeIndex;
	}

} /*}}}*/

/* unused function to print the binary of a block of data in dev
 */
void print_block(int dev, int block){ /*{{{*/
	int i,j;
	char arr[1024];
	lseek(dev, block*BLOCK_SIZE, SEEK_SET);
	for (i = 0; i < 1024; i++) {
		for (j = 0; j < 8; j++) {
			printf("%d ", arr[i] & 1 << j);
		}
	}
} /*}}}*/

/* takes in a command and checks it to the list of commands to return an integer
 * value to use in the switch statement in main
 */
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

/* the frontend to mymkdir, gets the pathname and finds the inode pointer of
 * the dirname parent and passes the pip along with the child dirname
 */
int make_dir(){ /*{{{*/
	char line[128], parent[64], child[64], temp[64];
	int dev, ino, r;
	MINODE *pip;

	printf("enter the pathname ");
	fgets(line, 128, stdin);
	line[strlen(line)-1]=0;
	if(line[0] == '/')
		dev = root->dev;
	else
		dev = running->cwd->dev;

	strcpy(temp, line);
	strcpy(parent, dirname(temp));

	strcpy(temp, line);
	strcpy(child, basename(temp));

	ino = getino(&dev, parent);
	pip = iget(dev, ino);

	r = my_mkdir(pip,child);

	return r;


} /*}}}*/

int my_mkdir(MINODE *pip, char *name){ /*{{{*/
	int inumber, bnumber, dev,i ;
	char *cp;
	MINODE *mip;
	DIR *dp;

	dev = pip->dev;

	inumber = ialloc(dev);
	bnumber = balloc(dev);

	mip = iget(dev, inumber);
	mip->INODE.i_block[0] = bnumber;

	for (i = 1; i < 16; i++) {
		mip->INODE.i_block[i]=0;
	}

	mip->dirty=1;
	mip->ino = inumber;
	mip->INODE.i_mode=0x41ED;
	mip->INODE.i_uid=running->uid;
	mip->INODE.i_gid=running->gid;
	mip->INODE.i_size=1024;
	mip->INODE.i_links_count=2;
	mip->INODE.i_atime=mip->INODE.i_ctime=mip->INODE.i_mtime = time(0L);
	mip->INODE.i_blocks=2;
	iput(mip);

	// write the . and .. entries into buff
	memset(buff, 0, 1024);

	dp = (DIR *)buff;
	dp->inode = inumber;
	strncpy(dp->name, ".", 1);
	dp->name_len = 1;
	dp->rec_len = 12;

	cp = buff;
	cp += dp->rec_len;
	dp = (DIR *)cp;

	dp->inode = pip->ino;
	dp->name_len = 2;
	strncpy(dp->name, "..", 2);
	dp->rec_len = BLOCK_SIZE - 12;
	put_block(dev, bnumber, buff);

	lseek(dev, pip->INODE.i_block[0]*BLOCK_SIZE, SEEK_SET );
	read(fd, buff, BLOCK_SIZE);

	cp = buff;
	dp = (DIR *) buff;

	while(cp+dp->rec_len < buff +1024) {
		cp += dp->rec_len;
		dp = (DIR *)cp;
	}
	// calculate size of last item in cwd to reduce rec_len
	int need_length = 4*((8+dp->name_len+3)/4);
	// storing the lenght of the new last dir
	int tmp = dp->rec_len - need_length;
	// change last dir rec_len to needed length 
	dp->rec_len = need_length;

	cp += dp->rec_len;
	dp = (DIR *)cp;

	dp->rec_len = tmp;
	dp->name_len = strlen(name);
	dp->inode = mip->ino;
	strncpy(dp->name, name, strlen(name));

	put_block(dev, pip->INODE.i_block[0], buff);

	pip->dirty = 1;
	pip->refCount++;
	pip->INODE.i_links_count++;
	pip->INODE.i_atime = time(0);
	iput(pip);
	return 0;


} /*}}}*/

unsigned long search(INODE *inodePtr, char *name) /*{{{*/
{
	DIR *dp = (DIR *) buff;
	int i,j;
	char *cp, tmp[256];
	for (i = 0; i < 12; i++) {
		cp = buff;
		dp = (DIR *) buff;
		lseek(fd, inodePtr->i_block[i]*BLOCK_SIZE, SEEK_SET);
		read(fd, buff, 1024);

		while(cp < buff + 1024){
			for (j = 0; j < dp->name_len; j++) {
				tmp[j]=(char)dp->name[j];
			}
			tmp[j] = 0;
			if(strcmp(name, tmp) == 0 ) {
				return dp->inode;
			}else{
				cp += dp->rec_len;
				dp = (DIR *)cp;
			}
		}
	}
	return 0;
} /*}}}*/

/* finds and allocates a free ibitmap bit to correspond to the inode blocks and 
 * returns the bit number
 */
// working dont fucking touch
unsigned long ialloc(int dev){ /*{{{*/
	int i;
	lseek(dev, BLOCK_SIZE*IBITMAP, SEEK_SET);

	get_block(dev, IBITMAP, buff);

	for (i = 1; i < 1024*8; i++) {
		if(tstbit(buff, i)==0){
			setbit(buff, i);
			put_block(dev, IBITMAP, buff);
			return (i+1);
		}
	}
	return 0;
} /*}}}*/

void idealloc(int dev, unsigned long ino) /*{{{*/
{
	int i;
	get_block(dev, IBITMAP, buff);
	clearbit(buff, ino-1);
	put_block(dev, IBITMAP, buff);
	return;
} /*}}}*/

// working dont fucking touch
unsigned long balloc( int dev ) /*{{{*/
{
	int iter = 0, ind = 0, pos = 0, spaceCount = 0, inodeCount = 0;
	char buf[1024];

	inodeCount = sp->s_blocks_count; // /	(BLOCK_SIZE*128);

	get_block( dev, (gp->bg_block_bitmap),(char*)&buf );

	for( iter = 0; iter < inodeCount; iter++  )
	{
		if (tstbit((char*)&buf,iter) ==0 )
		{
			setbit((char*)&buf,iter );
			put_block(dev,gp->bg_block_bitmap,(char*)&buf);
			printf("BALLOCRETURNING:%d\n",iter+1);
			return iter+1;
		}
	}

	return -1;
} /*}}}*/

//unsigned long balloc(int dev) /*{{{*/
/*{
  int i;
  char buf[1024];
  get_block(dev, BBITMAP, buf);

  for (i = 0; i < sp->s_blocks_count; i++) {
  if(tstbit(buf, i)==0){
  setbit(buf,i);
  put_block(dev, BBITMAP, buf);
  return (i);
  }
  }
  return 0;
  } /*}}}*/

void bdealloc(int dev, unsigned long ino) /*{{{*/
{
	int i;
	get_block(dev, BBITMAP, buff);
	clearbit(buff, ino-1);
	put_block(dev, BBITMAP, buff);
} /*}}}*/

void ls(char *pathname, PROC *parent) { /*{{{*/
	INODE *cwd = calloc(sizeof(INODE), 1);
	char path[128];
	strncpy(path, pathname, 128);
	int inodeIndex, seek;

	if(pathname[0] != '/')
		pathname[0] == '/';

	if(pathname[0] == '/') {
		strncpy(path, path+1, 127);
		char *token = strtok(path, "/");
		memcpy(cwd, &root->INODE,
				sizeof(INODE));

		while(token !=NULL) {
			inodeIndex =
				search(cwd, token);
			seek =
				((inodeIndex-1)/8 +
				 gp->bg_inode_table)*1024
				+
				(inodeIndex-1)%8 *
				128;

			lseek(fd,
					seek,SEEK_SET);

			read(fd,
					cwd,
					sizeof(INODE));
			token
				=
				strtok(NULL,
						"/");

		}
		printdir(cwd);
		return;

	}
	else
		if(pathname[0]
				<=
				0)
		{
			printdir(&parent->cwd->INODE);
			return;

		}
		else
		{


			char
				*token
				=
				strtok(path,
						"/");
			memcpy(cwd,
					&parent->cwd->INODE,
					sizeof(INODE));

			while(token
					!=NULL)
			{
				inodeIndex
					=
					search(cwd,
							token);
				seek
					=
					((inodeIndex-1)/8
					 +
					 gp->bg_inode_table)*1024
					+
					(inodeIndex-1)%8
					* 128;
				lseek(fd,
						seek,
						SEEK_SET);
				read(fd,
						cwd,
						sizeof(INODE));
				token
					=
					strtok(NULL,
							"/");
			}
		}
}
/*}}}*/

void printdir(INODE *inodePtr) /*{{{*/
{
	int	data_block=inodePtr->i_block[0];
	DIR
		*dp;
	lseek(fd,BLOCK_SIZE*data_block,SEEK_SET);
	read(fd,buff,BLOCK_SIZE);
	dp=(DIR *)buff;
	char
		*cp=buff;

	while(cp<buff+1024)
	{
		char name[dp->name_len];
		memcpy(name,dp->name,dp->name_len+1);
		name[dp->name_len]='\0';

		printf("%d\t%d\t%d\t%s\n",dp->inode,dp->rec_len,dp->name_len,name);
		cp+=dp->rec_len;
		dp=(DIR *)cp;
	}
	return;
}
/*}}}*/

void cd(char *pathname) /*{{{*/
{
	unsigned long ino = getino((int *)fd, pathname);
	MINODE *mp = iget(fd, ino);
	iput(running->cwd);
	running->cwd = mp;
	return;

} /*}}}*/



#endif
