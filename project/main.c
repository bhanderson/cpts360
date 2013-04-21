#include "header.h"

int main(int argc, const char *argv[])
{
	char line[128], command[32], path[64], filepath[64],scndpath[64];
	int cmd;

	init();

	printf("input diskimage: ");
	fgets(line, 128, stdin);
	line[strlen(line)-1] = 0; 		// null terminate line
	mount_root(line);

	while(1){
		printf("input command: ");
		fgets(line, 128, stdin);
		line[strlen(line)-1] = 0; // kill \r char at end
		if(line[0]==0)
			continue;
		path[0] = '\0';
		command[0]='\0';
		scndpath[0]='\0';
		sscanf(line, "%s %s %s", command, path,scndpath);
		cmd = findCmd(command);
		switch(cmd){ /*{{{*/
			case 0 :
				make_dir(path);
				break;
			case 1 :
				do_cd(path);
				break;
			case 2 :
				do_pwd();
				break;
			case 3 :
				ls(path, running);
				break;
			case 4 :
//				mount();
				break;
			case 5 :
//				umount(pathname);
				break;
			case 6 :
				creat_file(path);
				break;
			case 7 :
//				rmdir();
				break;
			case 8 :
//				rm_file();
				break;
			case 9 :
//				open_file();
				break;
			case 10:
//				close_file();
				break;
			case 11:
//				read_file();
				break;
			case 12:
//				write_file();
				break;
			case 13:
//				cat_file();
				break;
			case 14:
//				cp_file();
				break;
			case 15:
//				mv_file();
				break;
			case 16:
//				pfd();
				break;
			case 17:
//				lseek_file();
				break;
			case 18:
//				rewind_file();
				break;
			case 19:
				mystat(path);
				break;
			case 20:
//				pm();
				break;
			case 21:
//				menu();
				break;
			case 22:
//				access_file();
				break;
			case 23:
				mychmod(path);
				break;
			case 24:
//				chown_file();
				break;
			case 25:
//				cs();
				break;
			case 26:
//				do_fork();
				break;
			case 27:
//				do_ps();
				break;
			case 28:
//				do_kill();
				break;
			case 29:
//				quit();
				break;
			case 30:
				do_touch(path);
				break;
			case 31:
//				sync();
				break;
			case 32:
				do_link(path,scndpath);
				break;
			case 33:
				do_unlink(path);
				break;
			case 34:
//				symlink();
				break;
			case 99:
				printf("Exiting\n");
				exit(0);
				break;
			default:
				printf("invalid command\n");
				break;
		} /*}}}*/
	} // end infinate while
	return 0;
} // end main
