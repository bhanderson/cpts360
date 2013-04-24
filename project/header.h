#ifndef HEADER_H
#define HEADER_H

#include "types.h"


void init();
void mount_root(char *path);
MINODE *iget(int dev, unsigned long ino);
void iput(MINODE *mip);
void get_block(int dev, unsigned long block, char *buf);
void put_block(int dev, int block, char *buf);
unsigned long getino(int dev, char *pathname);
void print_block(int dev, int block);
int findCmd(char *cname);
int make_dir(char* path);
int my_mkdir(MINODE *pip, char *name);
unsigned long search(INODE *inodePtr, char *name);
unsigned long ialloc(int dev);
void idealloc(int dev, unsigned long ino);
unsigned long balloc(int dev);
void bdealloc(int dev, unsigned long ino);
void ls(char *pathname, PROC *parent);
void printdir(INODE *inodePtr);
int do_cd(char *pathname);
int do_pwd();
void pwd(MINODE *wd,int childIno);
void mystat(char *path);
int do_stat(char *path, struct stat *stPtr);
void do_touch(char *path);
void mychmod(char *path);
int deleteChild(MINODE* pip,char* name);
int creat_file(char *path);
int my_creat_file(MINODE *pip, char *name);
int do_link(char *oldpath, char *newpath);
int do_symlink(char *oldpath, char *newpath);
int do_rmdir(char* path);
int rm_file(char* path);
void deallocateInodeDataBlocks(int dev, MINODE* mip);
int tstbit(char *buf, int BIT);
int setbit(char *buf, int BIT);
int clearbit(char *buf, int BIT);
int open_file(char* path,char mode);
int falloc(OFT* oftp);
void do_truncate(int dev,MINODE *mip);
int close_file(int fd);
int pfd();
MINODE* findParent(MINODE* mip,MINODE* pip);
int printfilepath(MINODE* mip);
int lseek_file(int fd, long position);
int myread(int fd,char* m_buff,long nbytes);
int read_file(int fd, long bytes);
void my_cat(char *filename);
int write_file();
int mywrite(int fd, char *writebuff, int nbytes);
int do_unlink(char* path);
void mycp(char *src, char *dest);
void mymv(char *src, char *dest);
void exit_cleanup();
int chown_file(char* path,char* scndpath);
int rewind_file(int this_fd);


extern PROC *running;
extern PROC *p0;
extern PROC *p1;

extern MINODE *root;
extern MINODE minode[100];

extern int fd;

extern char buff[BLOCK_SIZE];


#endif
