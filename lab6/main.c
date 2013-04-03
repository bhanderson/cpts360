#include "types.h"

void init();
INODE *mount_root(char *p);
void ls(char *p, PROC *parent);
int search(INODE *inodePtr, char *name);
void printdir(INODE *inodePtr);

PROC *p0;
PROC *p1;
MINODE minode[100];
MINODE *root = 0;
int fd;
char buff[BLOCK_SIZE];
char path[64];
SUPER *sb;
GD    *gp;
INODE *in;


int main(int argc, char *argv[]) {
    int i,cmd;
    char line[128], cname[64];
	printf("%s",argv[1]);
    init();
	memcpy(&root->INODE,mount_root(argv[1]),sizeof(INODE));
	ls(argv[2],p0);
/*
    while(1) {
        printf("P%d running: ", running->pid);
        printf("input command : ");
        fgets(line, 128, stdin);
        line[strlen(line)-1] = 0;  // kill the \r char at end
        if (line[0]==0) continue;

        sscanf(line, "%s %s %64c", cname, pathname, parameter);

		cmd = findCmd(cname);

		switch(cmd) {
			case 0 :
				make_dir();
				break;
			case 1 :
				change_dir();
				break;
			case 2 :
				pwd(cwd);
				break;
			case 3 :
				list_dir();
				break;
			case 4 :
				mount();
				break;
			case 5 :
				umount(pathname);
				break;
			case 6 :
				creat_file();
				break;
			case 7 :
				rmdir();
				break;
			case 8 :
				rm_file();
				break;
			case 9 :
				open_file();
				break;
			case 10:
				close_file();
				break;

			case 11:
				read_file();
				break;
			case 12:
				write_file();
				break;
			case 13:
				cat_file();
				break;

			case 14:
				cp_file();
				break;
			case 15:
				mv_file();
				break;

			case 16:
				pfd();
				break;
			case 17:
				lseek_file();
				break;
			case 18:
				rewind_file();
				break;
			case 19:
				mystat();
				break;

			case 20:
				pm();
				break;

			case 21:
				menu();
				break;

			case 22:
				access_file();
				break;
			case 23:
				chmod_file();
				break;
			case 24:
				chown_file();
				break;

			case 25:
				cs();
				break;
			case 26:
				do_fork();
				break;
			case 27:
				do_ps();
				break;
			case 28:
				do_kill();
				break;

			case 29:
				quit();
				break;
			case 30:
				do_touch();
				break;

			case 31:
				sync();
				break;
			case 32:
				link();
				break;
			case 33:
				unlink();
				break;
			case 34:
				symlink();
				break;
			default:
            printf("invalid command\n");
            break;
        }
    }*/
    return 0;
} /* end main */
void init() {
    int i;
    p0 = malloc(sizeof(PROC));
    p0->cwd = malloc(sizeof(MINODE));
    //p0->cwd->INODE = malloc(sizeof(INODE));
    p1 = malloc(sizeof(PROC));
    p1->cwd = malloc(sizeof(MINODE));
    //p0->cwd->INODE = malloc(sizeof(INODE));
	root = malloc(sizeof(MINODE));
    p0->uid=0;
    //p0->cwd=0;
    p1->uid=1;
    //p0->cwd=0;
	sp = malloc(sizeof(SUPER));
	gp = malloc(sizeof(GD));
	in = malloc(sizeof(INODE));


    for (i=0; i<100; i++) {
        minode[i].refCount=0;
    }

    return;
}

INODE *mount_root(char *path) {

    fd = open(path,O_RDWR);
    lseek(fd, BLOCK_SIZE*SUPERBLOCK, SEEK_SET);
	read(fd, sb, BLOCK_SIZE);
	lseek(fd, BLOCK_SIZE*GDBLOCK, SEEK_SET);
	read(fd, gp, sizeof(GD));
	lseek(fd, BLOCK_SIZE*gp->bg_inode_table+sizeof(INODE), SEEK_SET);
	read(fd, in, sizeof(INODE));
	memcpy(&p0->cwd->INODE, in, sizeof(INODE));
	memcpy(&p1->cwd->INODE, in, sizeof(INODE));
	return in;
}


void ls(char *pathname, PROC *parent){
	INODE *cwd = calloc(sizeof(INODE), 1);
	char path[128];
	strncpy(path, pathname, 128);
	int inodeIndex;

	if(pathname[0] == '/'){
		strncpy(path, path+1, 127);
		char *token = strtok(path, "/");
		memcpy(cwd, &root->INODE, sizeof(INODE));

		while(token !=NULL){
			inodeIndex = search(cwd, token);
			lseek(fd, (BLOCK_SIZE * gp->bg_inode_table + (128 *
			inodeIndex)),SEEK_SET);

			read(fd, cwd, sizeof(INODE));
			token = strtok(NULL, "/");

		}
		printdir(cwd);
		return;

	} else if(pathname[0] == '\0'){
		printdir(&parent->cwd->INODE);
		return;

	}else{


		char *token = strtok(path, "/");
		memcpy(cwd, &parent->cwd->INODE, sizeof(INODE));

		while(token !=NULL){
			inodeIndex = search(cwd, token);
			lseek(fd, (BLOCK_SIZE * gp->bg_inode_table + (128 *
			inodeIndex)),SEEK_SET);

			read(fd, cwd, sizeof(INODE));
			token = strtok(NULL, "/");
		}
	}
}

int search(INODE *inodePtr, char *name){
	DIR *dp = (DIR *) buff;
	int i,j;
	char *cp;
	char tmp[256];
	for ( i =0; i <12; i ++){
		cp = buff;
		dp = (DIR *) buff;
		lseek(fd, inodePtr->i_block[i]*1024, SEEK_SET);
		read(fd, buff, 1024);
		while(cp < buff + 1024){
			for ( j = 0; j < dp->name_len; j++ ){
				tmp[j]=(char)dp->name[j];
			}
			tmp[j] = 0;
			if( strcmp(name, tmp) == 0 ){
				return dp->inode;
			}
			else
			{
				cp += dp->rec_len;
				dp = (DIR *)cp;
			}
		}
	}
	return 0;
}


void printdir(INODE *inodePtr){
	int data_block = inodePtr->i_block[0];
	DIR *dp;
	lseek(fd, BLOCK_SIZE*data_block, SEEK_SET);
	read(fd, buff, BLOCK_SIZE);
	dp = (DIR *)buff;
	char *cp = buff;

	while(cp < buff + 1024){
		char name[dp->name_len];
		memcpy(name, dp->name, dp->name_len+1);
		name[dp->name_len]='\0';

		printf("%d\t%d\t%d\t%s\n", dp->inode, dp->rec_len, dp->name_len, name);
		cp += dp->rec_len;
		dp = (DIR *)cp;
	}
	return;
}
