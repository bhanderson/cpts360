#include "header.h"
PROC *running;
PROC *p0;
PROC *p1;

MINODE *root;
MINODE minode[100];

int fd;

char buff[BLOCK_SIZE];
char read_buff[BLOCK_SIZE]; // A special buff for reads


void init() /*{{{*/
{
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
void mount_root( char *path ) /*{{{*/
{
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
MINODE *iget(int dev, unsigned long ino) /*{{{*/
{
	int i=0, freeNode=0, block=0, pos=0;
	freeNode=0;
	for (i = 0; i < 100; i++) {
		if(minode[i].ino == 0){
			if(freeNode == 0)
				freeNode = i;
		}
		if(minode[i].ino == ino){
			minode[i].refCount ++;
			//begin hacky test code
			/*			block = (ino-1) / INODES_PER_BLOCK + gp->bg_inode_table;
						pos = (ino-1) % INODES_PER_BLOCK;
						get_block(fd, block, (char *)&buff);
						memcpy(&minode[i].INODE, (((INODE*)buff) + pos),sizeof(INODE));
			 */           //end hacky test coe
			return &minode[i];
		}
	}

	// else it is not in the minode array so find it on the disk
	block = (ino-1) / INODES_PER_BLOCK + gp->bg_inode_table;
	pos = (ino-1) % INODES_PER_BLOCK;
	get_block(fd, block, (char *)&buff);

	minode[freeNode].dev = dev;
	minode[freeNode].dirty = 0;
	minode[freeNode].ino = ino;
	memcpy(&minode[freeNode].INODE, (((INODE*)buff) + pos),sizeof(INODE));
	minode[freeNode].mounted = 0;
	minode[freeNode].mountptr = NULL;
	minode[freeNode].refCount = 1;

	return &minode[freeNode];
} /*}}}*/

/* takes a minode * and writes it to fd
 */
void iput(MINODE *mip) /*{{{*/
{
	int block, pos;
	mip->refCount--;
	INODE *itmp;
	if(mip->refCount > 0)
		return;
	if(mip->dirty == 0)
		return;

    block = (mip->ino - 1) / INODES_PER_BLOCK + gp->bg_inode_table;
	pos = (mip->ino - 1) % INODES_PER_BLOCK;

	itmp = ( (INODE *)buff + pos);

	get_block(fd, block, (char *)&buff);
	memcpy( itmp, &mip->INODE, 128 );
	put_block(fd, block, buff);
	return;
} /*}}}*/

/* gets a block of data from dev and stores it to buf
 */
void get_block(int dev, unsigned long block, char *buf) /*{{{*/
{
	lseek(dev,(long)BLOCK_SIZE*block,SEEK_SET);
	read(dev, buf, BLOCK_SIZE);
} /*}}}*/

/* puts a block of data from buf and stores it to fd
 */
void put_block(int dev, int block, char *buf) /*{{{*/
{
	lseek(dev, BLOCK_SIZE*block, SEEK_SET);
	write(dev, buf, BLOCK_SIZE);
	return;
} /*}}}*/

/* traverses the pathname to the last point in pathaname and returns the inode
 * number of that inode
 * TODO error checking, and cases
 */
unsigned long getino(int dev, char *pathname) /*{{{*/
{
	INODE *cwd = malloc(sizeof(INODE));
	char path[128], *token;
	int inodeIndex, seek;
	strncpy(path, pathname, 128);

	if(pathname[0] == '/'){
		strncpy(path, path+1, 127); // remove root symbol
		token = strtok(path, "/");
		//cwd = &root->INODE;
		memcpy(cwd, &root->INODE, sizeof(INODE));

		while(token !=NULL){
			inodeIndex = search(cwd, token);
			if(inodeIndex == 0)
				return 0;
			seek = ((inodeIndex-1) / 8 + gp->bg_inode_table)*BLOCK_SIZE +
				(inodeIndex-1)%8 * 128;
			lseek(fd, seek, SEEK_SET);

			read(fd, cwd, sizeof(INODE));
			token = strtok(NULL, "/");
		}
		return inodeIndex;
	}else {
		token = strtok(path, "/");
		//cwd = &running->cwd->INODE;
		memcpy(cwd, &running->cwd->INODE, sizeof(INODE));

		while(token != NULL) {
			inodeIndex = search(cwd, token);
			if(inodeIndex == 0){
				perror("cannot find that inode");
				return 0;
			}
			seek = ((inodeIndex-1) / 8 + gp->bg_inode_table)*BLOCK_SIZE +
				(inodeIndex-1)%8 * 128;
			lseek(dev, seek, SEEK_SET);
			read(fd, cwd, sizeof(INODE));
			token = strtok(NULL, "/");
		}
		return inodeIndex;
	}

} /*}}}*/

/* unused function to print the binary of a block of data in dev
 */
void print_block(int dev, int block) /*{{{*/
{
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
int findCmd(char *cname) /*{{{*/
{
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
		return 24;
	if(strcmp(cname, "cs")==0)
		return 25;
	if(strcmp(cname, "fork")==0)
		return 26;
	if(strcmp(cname, "ps")==0)
		return 27;
	if(strcmp(cname, "kill")==0)
		return 28;
	if(strcmp(cname, "quit")==0)
		return 29;
	if(strcmp(cname, "touch")==0)
		return 30;
	if(strcmp(cname, "sync")==0)
		return 31;
	if(strcmp(cname, "link")==0)
		return 32;
	if(strcmp(cname, "unlink")==0)
		return 33;
	if(strcmp(cname, "symlink")==0)
		return 34;
	if(strcmp(cname, "exit")==0)
		return 99;
	return -1;
} /*}}}*/

/* the frontend to mymkdir, gets the pathname and finds the inode pointer of
 * the dirname parent and passes the pip along with the child dirname
 */
int make_dir(char* path) /*{{{*/
{
	char line[128], parent[64], child[64], temp[64];
	int dev, ino, r;
	MINODE *pip;

	//printf("enter the pathname ");
	//fgets(line, 128, stdin);
	//line[strlen(line)-1]=0;
	strcpy(line,path);
	if(line[0] == '/')
		dev = root->dev;
	else
		dev = running->cwd->dev;

	strcpy(temp, line);
	strcpy(parent, dirname(temp));

	strcpy(temp, line);
	strcpy(child, basename(temp));

	ino = getino(dev, parent);
	pip = iget(dev, ino);

	r = my_mkdir(pip,child);

	return r;


} /*}}}*/

int my_mkdir(MINODE *pip, char *name) /*{{{*/
{
    if (search(&pip->INODE,name)!= 0)
    {
        printf("Error: That filename already exists\n");
        return -1;
    }
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
	if (tmp>=need_length) //check to see if we need to allocate another data block
	{

        // change last dir rec_len to needed length
        dp->rec_len = need_length;

        cp += dp->rec_len;
        dp = (DIR *)cp;

        dp->rec_len = tmp;
        dp->name_len = strlen(name);
        dp->inode = mip->ino;
        strncpy(dp->name, name, strlen(name));

        put_block(dev, pip->INODE.i_block[0], buff);


	}
	else
	{
	    i = 0;
	    while (tmp<need_length){
	        i++;
            if (pip->INODE.i_block[i]==0)
            {
                pip->INODE.i_block[i]=balloc(dev);
                pip->refCount = 0;
                tmp = 1024;
                memset(buff, 0, 1024);
                cp = buff;
                dp = (DIR *) buff;
            }
            else{
                get_block(dev,pip->INODE.i_block[i],buff);
                cp = buff;
                dp = (DIR *) buff;

                while(cp+dp->rec_len < buff +1024)
                {
                    cp += dp->rec_len;
                    dp = (DIR *)cp;
                }
                // calculate size of last item in cwd to reduce rec_len
                need_length = 4*((8+dp->name_len+3)/4);
                // storing the lenght of the new last dir
                tmp = dp->rec_len - need_length;
                if (tmp>=need_length)
                {
                    dp->rec_len = need_length;
                    cp += dp->rec_len;
                    dp = (DIR *)cp;
                }

            }
	    }


        dp->rec_len = tmp;
        dp->name_len = strlen(name);
        dp->inode = mip->ino;
        strncpy(dp->name, name, strlen(name));


        put_block(dev, pip->INODE.i_block[i], buff);

    }
	pip->dirty = 1;
    pip->refCount++;
    //	pip->INODE.i_links_count++;
    pip->INODE.i_atime = time(0);
    iput(pip);
    return mip->ino;

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
				if(dp->inode == 0)
					return 0;
			}
		}
	}
	return 0;
} /*}}}*/

/* finds and allocates a free ibitmap bit to correspond to the inode blocks and
 * returns the bit number
 * working dont touch
 */
unsigned long ialloc(int dev) /*{{{*/
{
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
//Allocates a OFT pointer and assigns it to the next availble spot in the running process
int falloc(OFT* oftp) /*{{{*/
{
	int i = 0;
	for(i=0;i<10;i++)
	{
		if (running->fd[i]==NULL)
			break;
	}
	if (i==10)
	{
		return -1;
	}
	running->fd[i]=oftp;
	return i;
} /*}}}*/

/* deallocate the ino that was allocated
 */
void idealloc(int dev, unsigned long ino) /*{{{*/
{
	int i;
	get_block(dev, IBITMAP, buff);
	clearbit(buff, ino-1);
	put_block(dev, IBITMAP, buff);
	return;
} /*}}}*/

/* deallocates all of the dataBlocks for an inode */
void deallocateInodeDataBlocks(int dev, MINODE* mip) /*{{{*/
{
	char bitmap[1024],dblindbuff[1024];
	int i = 0;
	int j = 0;
	int indblk,dblindblk;
	unsigned long *indirect,*doubleindirect;
	get_block(dev,BBITMAP,bitmap);
	for ( i = 0; i<12; i++)
	{
		if (mip->INODE.i_block[i]!=0)
		{
			clearbit(bitmap, mip->INODE.i_block[i]-1);
			mip->INODE.i_block[i]=0;
		}
		else
		{
			put_block(dev,BBITMAP,bitmap);
			return ;
		}
	}
	// on to indirect blocks
	if (mip->INODE.i_block[i]!=0)
	{
		indblk = mip->INODE.i_block[i];
		get_block(dev,indblk,buff);
		indirect = (unsigned long *)buff;
		for (i=0;i<256;i++)
		{
			if(*indirect != 0)
			{
				clearbit(bitmap, *indirect-1);
				*indirect = 0;
				indirect++;
			}
			else
			{
				clearbit(bitmap, indblk-1);
				put_block(dev,indblk,buff);
				put_block(dev,BBITMAP,bitmap);
				mip->INODE.i_block[12] = 0;
				return;
			}
		}
	}
	else
	{
		put_block(dev,BBITMAP,bitmap);
		return;
	}
	//then double indirect
	if (mip->INODE.i_block[13]!=0)
	{
		dblindblk = mip->INODE.i_block[13];
		get_block(dev,dblindblk,dblindbuff);
		doubleindirect = (unsigned long *)dblindbuff;
		for (i=0;i<256;i++)
		{
			indblk = *doubleindirect;
			get_block(dev,indblk,buff);
			indirect = (unsigned long *)buff;
			for (j=0;j<256;j++)
			{
				if(*indirect != 0)
				{
					clearbit(bitmap, *indirect-1);
					*indirect = 0;
					indirect++;
				}
				else
				{
					clearbit(bitmap, indblk-1);
					clearbit(bitmap, dblindblk-1);
					put_block(dev,indblk,buff);
					put_block(dev,BBITMAP,bitmap);
					put_block(dev,dblindblk,dblindbuff);
					mip->INODE.i_block[13] = 0;
					return;
				}
				clearbit(bitmap, indblk-1);

			}
			doubleindirect++;
			if (*doubleindirect == 0)
			{//edge case handling
				clearbit(bitmap, indblk-1);
				clearbit(bitmap, dblindblk-1);
				put_block(dev,indblk,buff);
				put_block(dev,BBITMAP,bitmap);
				put_block(dev,dblindblk,dblindbuff);
				mip->INODE.i_block[13] = 0;
				return;
			}
		}
	}
	else
	{
		put_block(dev,BBITMAP,bitmap);
		return;
	}

} /*}}}*/

//an extension of the deallocate =InodeDataBlocksFunction
void do_truncate(int dev, MINODE *mip) /*{{{*/
{
	deallocateInodeDataBlocks(dev,mip);
	mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
	mip->INODE.i_size = 0;
	mip->dirty = 1;


} /*}}}*/

/* finds and allocates a free blockbitmap bit for data, returns the number found
 * working dont touch
 */
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

/* deallocate the ino that was allocated in block bitmap
 */
void bdealloc(int dev, unsigned long block) /*{{{*/
{
	int i;
	get_block(dev, BBITMAP, buff);
	clearbit(buff, block-1);
	put_block(dev, BBITMAP, buff);
} /*}}}*/

void ls(char *pathname, PROC *parent) /*{{{*/
{
	INODE *cwd = calloc(sizeof(INODE), 1);
	char path[128] = "\0";
	strncpy(path, pathname, 128);
	int inodeIndex, seek;
	// if ls fullpath
	if(pathname[0] == '/') {  /*{{{*/
		strncpy(path, path+1, 127);
		char *token = strtok(path, "/");
		memcpy(cwd, &root->INODE,sizeof(INODE));

		while(token !=NULL) {
			inodeIndex =search(cwd, token);
			if (inodeIndex==0) {
				printf("cannot find that directory\n");
				return;
			}
			seek =((inodeIndex-1)/8 + gp->bg_inode_table)*1024+(inodeIndex-1)%8
				*128;

			lseek(fd,seek,SEEK_SET);
			read(fd,cwd,sizeof(INODE));
			token=strtok(NULL,"/");
		}
		// check if path is a dir
		if((cwd->i_mode & 0040000) != 0040000){
			printf("cannot ls non dir");
			return;
		}

		printdir(cwd);
		return;
	} /*}}}*/

	// if ls cwd
	else if(pathname[0]	<=0){
		printf("current dir: Ino %lu, Iblock[0]= %lu\n",(long unsigned
					int)parent->cwd->ino, (long unsigned int)parent->cwd->INODE.i_block[0]);
		printdir(&parent->cwd->INODE);
		return;
	}
	// if ls local path
	else{
		char *token=strtok(path,"/");
		memcpy(cwd,	&parent->cwd->INODE,sizeof(INODE));

		while(token!=NULL){
			inodeIndex=search(cwd,token);
			if (inodeIndex==0) {
				printf("cannot find that directory\n");
				return;
			}
			seek=((inodeIndex-1)/8 + gp->bg_inode_table)*1024+
				(inodeIndex-1)%8* 128;
			lseek(fd,seek,SEEK_SET);
			read(fd,cwd,sizeof(INODE));
			printdir(cwd);
			token=strtok(NULL,"/");
		}
	}
}
/*}}}*/

void printdir(INODE *inodePtr) /*{{{*/
{
    int i=0;
    while ((i <12)&&(inodePtr->i_block[i]!=0))
    {
        int	data_block=inodePtr->i_block[i];
        DIR *dp;
        char fbuff[1024];
        memset(fbuff, 0, 1024);
        lseek(fd,BLOCK_SIZE*data_block,SEEK_SET);
        read(fd,fbuff,BLOCK_SIZE);
        dp=(DIR *)fbuff;
        char *cp=fbuff;
        MINODE *mip;
        int ino = dp->inode;
        char name[256];
        while(cp<fbuff+1024)
        {
            mip = iget(fd, ino);
            if(S_ISREG( mip->INODE.i_mode ) ) printf("r");
            if(S_ISDIR( mip->INODE.i_mode ) ) printf("d");
            if(S_ISLNK( mip->INODE.i_mode ) ) printf("l");
            //		printf("\n");
            // user permissions
            printf((mip->INODE.i_mode & 1 << 8) ? "r" : "-");
            printf((mip->INODE.i_mode & 1 << 7) ? "w" : "-");
            printf((mip->INODE.i_mode & 1 << 6) ? "x" : "-");
            // group permissions
            printf((mip->INODE.i_mode & 1 << 5) ? "r" : "-");
            printf((mip->INODE.i_mode & 1 << 4) ? "w" : "-");
            printf((mip->INODE.i_mode & 1 << 3) ? "x" : "-");
            // other permissions
            printf((mip->INODE.i_mode & 1 << 2) ? "r" : "-");
            printf((mip->INODE.i_mode & 1 << 1) ? "w" : "-");
            printf((mip->INODE.i_mode & 1 << 0) ? "x" : "-");
            char time_s[64];
            //char *time = time_s;
            //const time_t *t = (unsigned int)&mip->INODE.i_ctime;
            ctime_r((time_t *)&mip->INODE.i_mtime, time_s);
            time_s[strlen(time_s)-1]=0;
            printf(" %3d%3d %3d%6d %20s ", dp->inode, mip->INODE.i_uid,
                    mip->INODE.i_gid, mip->INODE.i_size, time_s);
            //		char name[dp->name_len+1];
            memmove(name, dp->name, dp->name_len);
            //		memcpy(name,dp->name,dp->name_len);
            name[dp->name_len]='\0';
            if(S_ISLNK( mip->INODE.i_mode)){
                printf("%16s->%s\n",name,(char *)mip->INODE.i_block);
            }else{
                printf("%16s\n",name);
            }
            iput(mip);
            cp+=dp->rec_len;
            dp=(DIR *)cp;
            ino = dp->inode;
        }
        i++;
    }
	return;
} /*}}}*/

int do_cd(char *pathname) /*{{{*/
{
	MINODE *mip;
	if(pathname[0]<=0){
		running->cwd = iget(root->dev, 2);
		return 0;
	}
	if(strcmp(pathname, "/") == 0){
		printf("CDing to root\n");
		running->cwd = iget(root->dev, 2);
		return 0;
	}
	int ino = getino(fd, pathname);
	if(ino==0){
		printf("directory doesn't exist\n");
		return 0;
	}
	mip = iget(root->dev, ino);
	//mip = iget (root->dev,ino);
	if((mip->INODE.i_mode & 0100000) == 0100000){
		iput(mip);
		printf("cannot cd to non dir\n");
		return -1;
	}
	running->cwd = mip;
	return 0;
} /*}}}*/

int do_pwd() /*{{{*/
{
	pwd(running->cwd,0);
	printf("\n");
	return 0;
} /*}}}*/

void pwd(MINODE *wd, int childIno) /*{{{*/
{
	if(wd->ino == root->ino){
		printf("/");
	}
	char buf[1024], *cp,name[64];
	DIR *dp;
	MINODE *parentmip;
	//int block = (mip->ino -1) / INODES_PER_BLOCK + gp->bg_inode_table;

	get_block(fd, wd->INODE.i_block[0], (char *)&buf);
	dp = (DIR *)buf; // get first dir "."
	cp = buf + dp->rec_len;
	dp = (DIR *)cp; // get second dir ".."
	if(wd->ino != root->ino){
		int ino = dp->inode; // get the inode number
		parentmip = iget(fd, ino);
		pwd(parentmip,wd->ino);
	}
	if (childIno!=0){
		while (dp->inode != childIno)
		{
			cp += dp->rec_len;
			dp = (DIR *)cp; // get second dir ".."
		}
		strncpy(name,dp->name,dp->name_len);
		name[dp->name_len] = '\0';
		printf("%s/",name);
	}

	return;
} /*}}}*/

//prints the currently open file descriptors and their modes
//TODO pretty up the formatting for long file names (currently the tab spacing goes off)
int pfd() /*{{{*/
{
	int i;
	printf("Filename\tFD\tmode\toffset\n");
	printf("--------\t--\t----\t------\n");
	for(i = 0;i<10;i++)
	{
		if (running->fd[i]!= NULL)
		{
			printfilepath(running->fd[i]->minodeptr);
			printf("\t\t%d\t",i);
			switch(running->fd[i]->mode)
			{
				case 0:
					printf("READ\t");
					break;
				case 1:
					printf("WRITE\t");
					break;
				case 2:
					printf("R/W\t");
					break;
				case 3:
					printf("APPEND\t");
					break;
				default:
					printf("??????\t");//this should never happen
					break;
			}
			printf("%li\n",running->fd[i]->offset);
		}
	}
	return 0;
} /*}}}*/

//prints the path to the current file
int printfilepath(MINODE* mip) /*{{{*/
{
	int ino = mip->ino;
	MINODE* pip = findParent(mip,root);
	pwd(pip,0);
	int i;
	DIR *dp;
	char* cp,name[64];
	for (i = 0; i < 12; i++) {
		get_block(fd,pip->INODE.i_block[i],buff);
		if ( pip->INODE.i_block[i]==0)
		{
			break;
		}
		cp = buff;
		dp = (DIR *) buff;

		//get rid of the first two entries or we will be stuck forever
		cp += dp->rec_len;
		dp = (DIR *)cp;
		cp += dp->rec_len;
		dp = (DIR *)cp;
		//Depth first search
		while(cp < buff + 1024){
			if (dp->inode == mip->ino)
			{
				strncpy(name,dp->name,dp->name_len);
				name[dp->name_len] = '\0';
				printf("%s",name);
			}
			cp += dp->rec_len;
			dp = (DIR *)cp;


		}
	}
	return 0;
} /*}}}*/
//Recusive depth first search function to find the parent
//THIS IS BAD, THERE MUST BE A BETTER WAY
MINODE* findParent(MINODE* mip,MINODE* pip) /*{{{*/
{
	MINODE* result;
	DIR *dp;
	char* cp;
	int i,j;
	char tmpbuff[1024];
	for (i = 0; i < 12; i++) {
		if ( pip->INODE.i_block[i]==0)
		{
			break;
		}
		cp = tmpbuff;
		dp = (DIR *) tmpbuff;


		lseek(fd, pip->INODE.i_block[i]*BLOCK_SIZE, SEEK_SET);
		read(fd, tmpbuff, 1024);
		//get rid of the first two entries or we will be stuck forever
		cp += dp->rec_len;
		dp = (DIR *)cp;
		cp += dp->rec_len;
		dp = (DIR *)cp;
		//Depth first search
		while(cp < tmpbuff + 1024){
			if (dp->inode == mip->ino)
				return pip;
			else
			{
				MINODE* cip = iget(fd, dp->inode);
				if (S_ISDIR( cip->INODE.i_mode))
				{
					result = findParent(mip,cip);
					if (result!=NULL)
						return result;
				}

			}

			cp += dp->rec_len;
			dp = (DIR *)cp;

		}
	}
	return NULL;
} /*}}}*/

void mystat(char *path){ /*{{{*/
	struct stat mystat;
	int r = do_stat(path, &mystat);
	return;
} /*}}}*/

int do_stat(char *pathname, struct stat *stPtr) /*{{{*/
{
	unsigned long ino = getino(fd, pathname);
	MINODE *mip = iget(fd, ino);
	//	memcpy(destination, source, size)
	stPtr->st_dev = fd;
	memcpy(&stPtr->st_ino, &ino, sizeof(ino_t));
	memcpy(&stPtr->st_mode, &mip->INODE.i_mode, sizeof(mode_t));
	memcpy(&stPtr->st_nlink, &mip->INODE.i_links_count, sizeof(nlink_t));
	memcpy(&stPtr->st_uid, &mip->INODE.i_uid, sizeof(uid_t));
	memcpy(&stPtr->st_gid, &mip->INODE.i_gid, sizeof(gid_t));
	memcpy(&stPtr->st_size, &mip->INODE.i_size, sizeof(off_t));
	stPtr->st_blksize = 1024;
	memcpy(&stPtr->st_blocks, &mip->INODE.i_blocks, sizeof(blkcnt_t));
	memcpy(&stPtr->st_atime, &mip->INODE.i_atime, sizeof(time_t));
	memcpy(&stPtr->st_mtime, &mip->INODE.i_mtime, sizeof(time_t));
	memcpy(&stPtr->st_ctime, &mip->INODE.i_ctime, sizeof(time_t));
	printf("dev: %d\t", (int)stPtr->st_dev);
	printf("ino: %u\t\t", (int)stPtr->st_ino);
	printf("mode: %u\t", (unsigned short)stPtr->st_mode);
	printf("nlink: %lu\t", (unsigned long)stPtr->st_nlink);
	printf("uid: %u\t", (int)stPtr->st_uid);
	printf("\n");
	printf("gid: %u\t", (int)stPtr->st_gid);
	printf("size: %d\t", (int)stPtr->st_size);
	printf("blksize: %d\t", (int)stPtr->st_blksize);
	printf("blocks: %lu\t", (unsigned long)stPtr->st_blocks);
	char *time_string = ctime(&stPtr->st_ctime);
	printf("\nctime: %s", time_string);
	time_string = ctime(&stPtr->st_atime);
	printf("atime: %s", time_string);
	time_string = ctime(&stPtr->st_mtime);
	printf("mtime: %s", time_string);
	printf("\n");

	iput(mip);
	return 0;
} /*}}}*/

void do_touch(char *path) /*{{{*/
{
	unsigned long ino = getino(fd, path);
	MINODE *mip = iget(fd, ino);

	mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
	iput(mip);
} /*}}}*/

void mychmod(char *pathname) /*{{{*/
{
	char line[128];
	int mode=0, i=0;
	char perm[64];
	for (i = 0; i < 64; i++) {
		perm[i]=0;
	}
	unsigned long ino = getino(fd, pathname);
	MINODE *mip = iget(fd, ino);
	printf("input the new mode: ");
	fgets(line, 128, stdin);
	line[strlen(line)-1]=0;
	sscanf(line, "%s", perm);
	printf("\nprevious permissions: %o\n", mip->INODE.i_mode);
	mode = perm[0]-48 << 6;
	mode |= perm[1]-48 << 3;
	mode |= perm[2]-48;
	printf("mode %d\n", mode);
	mip->INODE.i_mode &= 0xFF000;
	mip->INODE.i_mode |= mode;
	printf("new permissions: %o\n", mip->INODE.i_mode);
	return;
} /*}}}*/

int creat_file(char *path) /*{{{*/
{
	char line[128], parent[64], child[64], temp[64];
	int dev, ino, r;
	MINODE *pip;

	strcpy(line,path);
	if(line[0] == '/')
		dev = root->dev;
	else
		dev = running->cwd->dev;

	strcpy(temp, line);
	strcpy(parent, dirname(temp));

	strcpy(temp, line);
	strcpy(child, basename(temp));

	ino = getino(dev, parent);
	pip = iget(dev, ino);

	if (search(&pip->INODE,child)!= 0)
    {
        printf("Error: That filename already exists\n");
        return -1;
    }

	r = my_creat_file(pip,child);

	return r;


} /*}}}*/

// 0 read 1 write 2 read/write 3 append
int open_file(char* path,char mode) /*{{{*/
{
	if ((path[0]=='\0')||(mode=='\0'))
	{
		printf("SYNTAX: open [path] [mode (as int)]\n");
		printf("Modes: 0 = READ, 1 = WRITE, 2 = READ/WRITE, 3 = APPEND\n");
		return -1;
	}
	int ino = getino(fd,path);
	int i;
	int index;
	MINODE * mip = iget(fd,ino);
	if (!S_ISREG( mip->INODE.i_mode ))
	{
		printf("Error: file must be regular\n");
		return -1;
	}
	//check to see whether it is already open with an incompatable type
	for(i =0;i<10;i++)
	{
		if (running->fd[i] != 0)
		{
			if (running->fd[i]->minodeptr == mip)
			{
				if (running->fd[i]->mode>0)
				{
					printf("Error: File already opened for writing\n");
					return -1;
				}
			}
		}
	}
	OFT* oftp;
	oftp = malloc(sizeof(OFT));
	index = falloc(oftp);
	if (index == -1)
	{
		printf("Error: No open file descriptor slots in running process \n");
		return -1;
	}
	oftp->mode = mode-48;
	oftp->refCount = 1;
	oftp->minodeptr = mip;
	switch(oftp->mode)
	{
		case 0:
			oftp->offset = 0;
			break;
		case 1:
			oftp->offset = 0;
			break;
		case 2:
			oftp->offset = 0;
			break;
		case 3:
			oftp->offset = mip->INODE.i_size;
			break;
		default:
			printf("invalid mode\n");
			running->fd[index]=0;
			return -1;
	}
	mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
	mip->dirty = 1;
	return index;




} /*}}}*/
//seeks to a certain offset in an open file
int lseek_file(int fd, long position) /*{{{*/
{
	if (fd==-48)
	{
		printf("SYNTAX: lseek [fd (as int)] [offset (as long)]\n");
		return -1;
	}
	if (position>running->fd[fd]->minodeptr->INODE.i_size)
	{
		printf("ERROR: File length overrun\n");
		return -1;
	}
	else if(position<0)
	{
		printf("ERROR: Cannot Lseek a negitive number\n");
		return -1;
	}
	long original = running->fd[fd]->offset;
	running->fd[fd]->offset = position;
	return original;
} /*}}}*/


//closes a file with the specified file descriptor
int close_file(int fd) /*{{{*/
{
	MINODE* mip;
	if (fd==-48)
	{
		printf("SYNTAX: close [fd (as int)]\n");
		return -1;
	}
	if ((fd>=10)||(fd<0))
	{
		printf("Error: Invalid File descriptor \n");
		return -1;
	}
	if (running->fd[fd]==NULL)
	{
		printf("Error: File descriptor not found \n");
		return -1;
	}
	OFT* oftp = running->fd[fd];
	running->fd[fd] = 0;
	oftp->refCount--;
	if (oftp->refCount > 0)
		return 0;
	else //we are the last user of this oft entry
	{
		mip = oftp->minodeptr;
		iput(mip);
	}
	return 0;




} /*}}}*/
//the handler for user input into read
int read_file(int fd, long bytes) /*{{{*/
{
	if ((fd>=10)||(fd<0))
	{
		printf("SYNTAX: read [fd (as int)] nbytes\n");
		return -1;
	}
	if(running->fd[fd]==NULL)
	{
		printf("Error: must select a valid file descriptor\n");
		return -1;
	}
	if((running->fd[fd]->mode!=0)&&(running->fd[fd]->mode!=2))
	{
		printf("Error: file must be opened for read\n");
		return -1;
	}
	return myread(fd,read_buff,bytes);
} /*}}}*/
// reads nbtytes from a file specified by fd in to buffer mbuff
int myread(int fd,char* m_buff,long nbytes) /*{{{*/
{
	int flag = 0;
	int doubleflag = 0;
	char indir_buff[1024],dblindir_buff[1024];
	long * indirect;
	long * dblindirect;
	long superflag = 0;
	long indblk,dblindblk;
	long size = running->fd[fd]->minodeptr->INODE.i_size - running->fd[fd]->offset;
	long lblk,startByte,blk;
	int count = 0;
	while ((nbytes>0)&&(size>0))
	{
		lblk = running->fd[fd]->offset / BLOCK_SIZE;
		startByte = running->fd[fd]->offset % BLOCK_SIZE;
		if (lblk < 12)
		{
			blk = running->fd[fd]->minodeptr->INODE.i_block[lblk];
		}
		else if ((lblk >= 12)&&(lblk<256+12))
		{
			if (!flag)//we dont want to get_block for every time we switch lblks, that would be dumb
			{
				get_block(running->fd[fd]->minodeptr->dev,running->fd[fd]->minodeptr->INODE.i_block[13],indir_buff);
				flag = 1;
			}
			indirect = (long *)buff;
			blk = *(indirect+(lblk-12));

		}
		else
		{
			if (!doubleflag)
			{
				get_block(running->fd[fd]->minodeptr->dev,running->fd[fd]->minodeptr->INODE.i_block[14],dblindir_buff);
				doubleflag = 1;
			}
			dblindirect = (long *)dblindir_buff;
			if (superflag != *(dblindirect+((lblk-268) /256)))
			{
				superflag = *(dblindirect+((lblk-268) /256));
				get_block(running->fd[fd]->minodeptr->dev,superflag,indir_buff);
			}
			indirect = (long *)buff;
			blk = *(indirect+((lblk-268)%256));
		}
		get_block(running->fd[fd]->minodeptr->dev,blk,read_buff);
		char *cq = buff;
		char *cp = read_buff +startByte;
		int remain = BLOCK_SIZE - startByte;

		while (remain > 0)
		{
			*cq++ = *cp++;
			running->fd[fd]->offset++;
			count++;
			size--; nbytes--; remain--;
			if ((nbytes <= 0)||(size<=0))
				break;
		}
	}
	//printf("\nmyread : read %d char from file %d\n", count,fd);
	return count;
} /*}}}*/

int my_creat_file(MINODE *pip, char *name) /*{{{*/
{
	int inumber, bnumber, dev,i ;
	int flag = 0;
	char *cp;
	MINODE *mip;
	DIR *dp;

	dev = pip->dev;

	inumber = ialloc(dev);

	mip = iget(dev, inumber);

	for (i = 0; i < 15; i++) {
		mip->INODE.i_block[i]=0;
	}

	mip->dirty=1;
	mip->ino = inumber;
	mip->INODE.i_mode=0x81A4;
	mip->INODE.i_uid=running->uid;
	mip->INODE.i_gid=running->gid;
	mip->INODE.i_size=0;
	mip->INODE.i_links_count=0;
	mip->INODE.i_atime=mip->INODE.i_ctime=mip->INODE.i_mtime = time(0L);
	mip->INODE.i_blocks=2;
	iput(mip);

	// write the . and .. entries into buff
	memset(buff, 0, 1024);

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
	if (tmp>=need_length) //check to see if we need to allocate another data block
	{

        // change last dir rec_len to needed length
        dp->rec_len = need_length;

        cp += dp->rec_len;
        dp = (DIR *)cp;

        dp->rec_len = tmp;
        dp->name_len = strlen(name);
        dp->inode = mip->ino;
        strncpy(dp->name, name, strlen(name));

        put_block(dev, pip->INODE.i_block[0], buff);


	}
	else
	{
	    i = 0;
	    while (tmp<need_length){
	        i++;
            if (pip->INODE.i_block[i]==0)
            {
                pip->INODE.i_block[i]=balloc(dev);
                pip->refCount = 0;
                tmp = 1024;
                memset(buff, 0, 1024);
                cp = buff;
                dp = (DIR *) buff;
            }
            else{
                get_block(dev,pip->INODE.i_block[i],buff);
                cp = buff;
                dp = (DIR *) buff;

                while(cp+dp->rec_len < buff +1024)
                {
                    cp += dp->rec_len;
                    dp = (DIR *)cp;
                }
                // calculate size of last item in cwd to reduce rec_len
                need_length = 4*((8+dp->name_len+3)/4);
                // storing the lenght of the new last dir
                tmp = dp->rec_len - need_length;
                if (tmp>=need_length)
                {
                    dp->rec_len = need_length;
                    cp += dp->rec_len;
                    dp = (DIR *)cp;
                }

            }
	    }


        dp->rec_len = tmp;
        dp->name_len = strlen(name);
        dp->inode = mip->ino;
        strncpy(dp->name, name, strlen(name));


        put_block(dev, pip->INODE.i_block[i], buff);

    }
	pip->dirty = 1;
    pip->refCount++;
    //	pip->INODE.i_links_count++;
    pip->INODE.i_atime = time(0);
    iput(pip);
    return mip->ino;


} /*}}}*/

/* unlinks a file, if the file is the last link, deletes the file and unallocates it's datablock and inode */
int do_unlink(char* path) /*{{{*/
{
	//check for user error
	if (path[0]=='\0'){
		printf("Syntax: unlink [path]\n");
		return -1;
	}
	char parentdir[64],name[64], *cp, *endcp,*last;
	DIR * dp;
	MINODE * pip,*targetip;
	int parent, target;
	cp = strrchr(path, '/');
	if (cp == NULL){
		parent = running->cwd->ino; // same dir
		strcpy(name,path);
	}
	else{
		//this
		*(cp) = '\0';
		strcpy(parentdir, path);
		parent = getino(fd,parentdir);
		strcpy(name,cp+1);
	}
	target = getino(fd,path);
	if ((target==0)||(parent==0)){
		printf("Error: File must exist\n");
		return -1;
	}
	pip = iget(fd,parent);
	targetip = iget(fd,target);
	//check to make sure its not a dir
	if((targetip->INODE.i_mode & 0100000) != 0100000){
		iput(pip);
		printf("Error: Cannot unlink NON-REG files\n");
		return -1;
	}
	//decrement the i links count by one
	targetip->INODE.i_links_count--;
	if (targetip->INODE.i_links_count == 0)
	{
	    do_truncate(fd,targetip);
	}
	//increment refcount?
	targetip->refCount++;
	targetip->dirty=1;
	iput(targetip);
	idealloc(fd,targetip->ino);

	return deleteChild(pip,name);




} /*}}}*/

/* Creates a hard link to a file (NOT A FOLDER)
 */
int do_link(char* oldpath,char* newpath) /*{{{*/
{
    int i;
	//check for user error
	if ((oldpath[0]=='\0')||(newpath[0]=='\0')){
		printf("Syntax: link [oldpath] [newpath]\n");
		return -1;
	}
	//get the parent directory we are gonna be putting this into
	char parentdir[64],name[64], *cp;
	DIR * dp;
	MINODE * pip,*targetip;
	int parent;
	cp = strrchr(newpath, '/');
	if (cp == NULL){
		parent = running->cwd->ino; // same dir
		strcpy(name,newpath);
	}
	else{
		//this
		*(cp) = '\0';
		strcpy(parentdir, newpath);
		parent = getino(fd,parentdir);
		strcpy(name,cp+1);
	}
	//get the ino
	int target = getino(fd,oldpath);
	//check to make sure both inos exist, if not, terminate
	if ((target==0)||(parent==0)){
		printf("Both the target and the parent directory must exist\n");
		return -1;
	}
	//get the parent inode
	pip = iget(fd,parent);
    if (search(&pip->INODE,name)!= 0)
    {
        printf("Error: That filename already exists\n");
        return -1;
    }
	targetip = iget(fd,target);
	//check to make sure its not a dir
	if((targetip->INODE.i_mode & 0100000) != 0100000){
		iput(pip);
		printf("cannot link to a dir\n");
		return -1;
	}
	get_block(fd,pip->INODE.i_block[0],buff);

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
	if (tmp>=need_length) //check to see if we need to allocate another data block
	{

        // change last dir rec_len to needed length
        dp->rec_len = need_length;

        cp += dp->rec_len;
        dp = (DIR *)cp;

        dp->rec_len = tmp;
        dp->name_len = strlen(name);
        dp->inode =targetip->ino;
        strncpy(dp->name, name, strlen(name));

        put_block(fd, pip->INODE.i_block[0], buff);


	}
	else
	{
	    i = 0;
	    while (tmp<need_length){
	        i++;
            if (pip->INODE.i_block[i]==0)
            {
                pip->INODE.i_block[i]=balloc(fd);
                pip->refCount = 0;
                tmp = 1024;
                memset(buff, 0, 1024);
                cp = buff;
                dp = (DIR *) buff;
            }
            else{
                get_block(fd,pip->INODE.i_block[i],buff);
                cp = buff;
                dp = (DIR *) buff;

                while(cp+dp->rec_len < buff +1024)
                {
                    cp += dp->rec_len;
                    dp = (DIR *)cp;
                }
                // calculate size of last item in cwd to reduce rec_len
                need_length = 4*((8+dp->name_len+3)/4);
                // storing the lenght of the new last dir
                tmp = dp->rec_len - need_length;
                if (tmp>=need_length)
                {
                    dp->rec_len = need_length;
                    cp += dp->rec_len;
                    dp = (DIR *)cp;
                }

            }
	    }


        dp->rec_len = tmp;
        dp->name_len = strlen(name);
        dp->inode = targetip->ino;
        strncpy(dp->name, name, strlen(name));


        put_block(fd, pip->INODE.i_block[i], buff);

    }
	pip->dirty = 1;
    pip->refCount++;
    //	pip->INODE.i_links_count++;
    pip->INODE.i_atime = time(0);
    iput(pip);
    return targetip->ino;


} /*}}}*/

int do_symlink(char *oldpath, char *newpath){ /*{{{*/
	if(oldpath[0]==0 || newpath[0]==0){
		printf("Syntax symlink [oldpath] [newpath]\n");
		return -1;
	}
	if(oldpath[0]!='/'){
		printf("Oldpath must be absolute path!\n");
		return -1;
	}
	char parentdir[64], name[64], *cp;
	DIR *dp;
	MINODE *pip, *targetip;
	int parent;
	cp = strrchr(newpath, '/');
	if(cp == NULL){// not absolute path
		parent = running ->cwd->ino;
		strcpy(name, newpath);
	}else{
		*(cp) = '\0';
		strcpy(parentdir, newpath);
		parent = getino(fd, parentdir);
		strcpy(name, cp+1);
	}
	int target = creat_file(newpath);
	pip = iget(fd, parent);
	targetip = iget(fd, target);
	pip->dirty = 1;

	pip->refCount++;
	pip->INODE.i_links_count++;
	pip->INODE.i_atime = time(0);

	iput(pip);

	targetip->dirty = 1;
	targetip->refCount++;
	targetip->INODE.i_links_count++;
	targetip->INODE.i_mode = 0xA1A4;
	targetip->INODE.i_size = strlen(oldpath);
	memcpy(targetip->INODE.i_block, oldpath, strlen(oldpath));
	iput(targetip);

	return 0;
} /*}}}*/

int rm_file(char* path) /*{{{*/
{

	//check for user error
	if (path[0]=='\0'){
		printf("Syntax: rm [path]\n");
		return -1;
	}
	char parentdir[64],name[64], *cp, *endcp,*last;
	DIR * dp;
	MINODE * pip,*targetip;
	int parent, target;
	cp = strrchr(path, '/');
	if (cp == NULL){
		parent = running->cwd->ino; // same dir
		strcpy(name,path);
	}
	else{
		//this
		*(cp) = '\0';
		strcpy(parentdir, path);
		parent = getino(fd,parentdir);
		strcpy(name,cp+1);
	}
	target = getino(fd,path);
	if ((target==0)||(parent==0)){
		printf("Error: File must exist\n");
		return -1;
	}
	pip = iget(fd,parent);
	targetip = iget(fd,target);
	if((targetip->INODE.i_mode & 0100000) != 0100000){
		iput(pip);
		printf("Error: cannot rm a non regular file\n");
		return -1;
	}
	do_truncate(fd,targetip);
	iput(targetip);
	idealloc(fd,targetip->ino);
	deleteChild(pip,name);
	return 0;
} /*}}}*/

/* deletes an empty folder */
int do_rmdir(char* path) /*{{{*/
{
	//check for user error
	if (path[0]=='\0'){
		printf("Syntax: rmdir [path]\n");
		return -1;
	}
	char parentdir[64],name[64], *cp, *endcp,*last;
	DIR * dp;
	MINODE * pip,*targetip;
	int parent, target;
	cp = strrchr(path, '/');
	if (cp == NULL){
		parent = running->cwd->ino; // same dir
		strcpy(name,path);
	}
	else{
		//this
		*(cp) = '\0';
		strcpy(parentdir, path);
		parent = getino(fd,parentdir);
		strcpy(name,cp+1);
	}
	target = getino(fd,path);
	if ((target==0)||(parent==0)){
		printf("Error: File must exist\n");
		return -1;
	}
	pip = iget(fd,parent);
	targetip = iget(fd,target);
	if((targetip->INODE.i_mode & 0040000) != 0040000){
		iput(pip);
		printf("Error: cannot rmdir a non dir\n");
		return -1;
	}
	//check to make sure the dir is empty, we do this by checking the folder entry for .. and seeing if it's rec length is 1012
	get_block(fd,targetip->INODE.i_block[0],buff);
	cp = buff;
	dp = (DIR*) cp;
	cp += dp->rec_len;
	dp = (DIR*) cp;
	if (dp->rec_len != 1012)
	{
		printf("Errpr: Dir must be empty \n");
		return -1;
	}

	do_truncate(fd,targetip);
	iput(targetip);
	idealloc(fd,targetip->ino);
	//TODO Deallocate DIR Inode

	//Then we have to delete the directory entry
	deleteChild(pip,name);


	return 0;
} /*}}}*/

/* Deletes the child of a minode with name name */
int deleteChild(MINODE* pip,char* name){ /*{{{*/
	get_block(fd,pip->INODE.i_block[0],buff);

	char* cp = buff;
	DIR* dp = (DIR *) buff;
	int tmp,i,flag,last;
	last = 0;
	i =  0;
	char* endcp = buff;
	//find the item at the end of the buffer
	while(endcp+dp->rec_len < buff +1024) {
		endcp += dp->rec_len;
		dp = (DIR *)endcp;
	}
	dp =(DIR *) cp;
	while (cp < buff+1024)
	{
		if (dp->name_len == strlen(name))
		{
			if (strncmp(name,dp->name,dp->name_len)==0)
			{
				//do file delete operation
				tmp = dp->rec_len;
				if (cp == endcp)//if the item we are deleting is at the end we need to look at the last item and increase its rec_length
				{
					dp = (DIR *) last;
					dp->rec_len += tmp;
					break;
				}
				else
				{
					dp = (DIR *) endcp;
					dp->rec_len += tmp;
					//copy 1024 - current position - record length bytes from the end of the record to the end of the buffer over the current record
					memcpy(cp,cp+tmp,1024 - i - tmp);
				}
				//we wiped out the value of dp->rec_len so we need to use tmp
				break;
			}
		}
		last = (int)cp;
		i += dp->rec_len;
		cp += dp->rec_len;
		dp = (DIR *)cp;
	}
	put_block(fd,pip->INODE.i_block[0],buff);

	return 0;
} /*}}}*/

int tstbit(char *buf, int BIT) { /*{{{*/
	int i, j;
	i = BIT / 8;
	j = BIT % 8;
	return buf[i] & (1 << j);
} /*}}}*/

int setbit(char *buf, int BIT) { /*{{{*/
	int i, j;
	i = BIT / 8;
	j = BIT % 8;
	buf[i] |= (1 << j);
	return 1;
} /*}}}*/

int clearbit(char *buf, int BIT) { /*{{{*/
	int i, j;
	i = BIT / 8;
	j = BIT % 8;
	buf[i] &= ~(1 << j);
	return 1;
} /*}}}*/

void my_cat(char *filename) /*{{{*/
{
	int catfd = open_file(filename, '0');
	if(catfd == -1){
		printf("file is open already!!\n");
		return;
	}
	char dummy =0;
	int n=0;
	while( (n=read_file(catfd, 1024))){
		read_buff[n]=0;
		printf("%s", read_buff);
	}
	printf("\n");
	close_file(catfd);
} /*}}}*/

int write_file(char *pfd, char *pstring) /*{{{*/
{
	int writefd = pfd[0] - 48;
	if (writefd<0||writefd>9) {
		printf("you done goofed syntax: write [fd#]\n");
		return -1;
	}
	char string[1024];
	printf("Enter the string you want to write: ");
	fgets(string, 1024, stdin);
	string[strlen(string)-1]=0;

	//	strncpy(string, pstring, strlen(pstring));
	//	string[strlen(pstring)]=0;
	//	memcpy(string, pstring, strlen(pstring));
	if(running->fd[writefd]->mode == 0){
		printf("error: fd not open for write\n");
		return -1;
	}
	int nbytes = strlen(string);
	return(mywrite(writefd, string, nbytes));

} /*}}}*/

int mywrite(int fd, char *fbuf, int nbytes) /*{{{*/
{
	char wbuf[1024];
	int flag =0;
	int doubleflag = 0;
	char indir_buff[1024], dblindir_buff[1024];
	long *indirect;
	long *dblindirect;
	long superflag =0;
	long indblk, dblindblk;
	long size = running->fd[fd]->minodeptr->INODE.i_size - running->fd[fd]->offset;
	long lbk, startByte, blk;
	int count = 0, remain=0;
	char *cp, *cq;
	OFT *oftp = running->fd[fd];
	MINODE *mip = running->fd[fd]->minodeptr;
	cq = (char *)fbuf;

	while(nbytes > 0){
		lbk = oftp->offset / BLOCK_SIZE;
		startByte = oftp->offset % BLOCK_SIZE;
		if(lbk < 12){
			if(mip->INODE.i_block[lbk] == 0) // no data block yet
				mip->INODE.i_block[lbk] = balloc(mip->dev);
			blk = mip->INODE.i_block[lbk];
		} else if(lbk >= 12 && lbk < 256+12){
			if(!flag){
				get_block(mip->dev,mip->INODE.i_block[13],indir_buff);
				flag = 1;
			}
			indirect = (long *)buff;
			blk = *(indirect+(lbk-12));

		}else{
			// double indirect blocks
			if(!doubleflag){
				get_block(mip->dev,mip->INODE.i_block[14],dblindir_buff);
				doubleflag =1;
			}
			dblindirect = (long *)dblindir_buff;
			if(superflag != *(dblindirect+((lbk-268) /256))){
				superflag = *(dblindirect+((lbk-268) /256));
				get_block(mip->dev,superflag,indir_buff);
			}
			indirect = (long *)buff;
			blk = *(indirect+((lbk-268)%256));

		}
		get_block(mip->dev, blk, wbuf);
		if(wbuf[0]==0)
			memset(wbuf, 1024, 0);
		cp = wbuf + startByte;
		remain = BLOCK_SIZE - startByte;

		while (remain > 0){
			*cp++ = *cq++;
			nbytes--;
			remain--;
			oftp->offset++;
			if(oftp->offset > mip->INODE.i_size)
				mip->INODE.i_size++;

			if(nbytes <=0)
				break;
		}
		put_block(mip->dev, blk, wbuf);

	}

	mip->dirty = 1;
	iput(mip);
	printf("write %d char into file fd=%d\n",nbytes, fd);
	return nbytes;
} /*}}}*/

void mycp(char *src, char *dest){ /*{{{*/
	int srcfd = open_file(src, '0');
	int n;
	char cpbuf[1024];
	memset(cpbuf, 0,1024);
	if(srcfd == -1)
		return;
	int destino = creat_file(dest);
//	if(destino==-1)
//		return;
	int gd = open_file(dest, '1');
	if(gd ==-1)
		return;
	while( (n=myread(srcfd, cpbuf, 1024) ) ){
		memcpy(cpbuf, buff, n);
		mywrite(gd, cpbuf, n);
	}
	close_file(gd);
	close_file(srcfd);
	return;
} /*}}}*/

void mymv(char *src, char *dest){
	int srcfd = open_file(src, '0');
//	int destino = creat_file(dest);
//	int destfd = open_file(dest, '1');
	if(srcfd == -1)
		return;
	MINODE *smip = running->fd[srcfd]->minodeptr;
	
	if(smip->dev == fd){//same dev as src
		do_link(src,dest);
		do_unlink(src);
	}else{// not same dev
		close_file(srcfd);
		mycp(src,dest);
		do_unlink(src);
	}
	return;
}
