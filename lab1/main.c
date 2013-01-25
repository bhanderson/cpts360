#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


struct partition{
	unsigned char drive;
	unsigned char head;
	unsigned char sector;
	unsigned char cylinder;
	unsigned char sys_type;
	unsigned char end_head;
	unsigned char end_sector;
	unsigned char end_cylinder;
	unsigned int start_sector;
	unsigned int nr_sectors;

};

int fd;
int sector_one_start=0, estart=0, part =0;
char buf[512];
char *systems [0xff][20];

void fill_array(void);
void read_partitions(struct partition *p);


int main(void){
	fill_array();
	struct partition *p;
	char command;
	printf("Enter a command: (q to quit)");
	command = getchar();
	if(command == 'p'){
		puts("Device \t Type \t Head \t Start Sector \t    Size \t System\n");
		fd = open("/home/bryce/workspace/lab1/vdiskImage", O_RDONLY);
		lseek(fd, (long)0, 0);
		read(fd, buf, 512);
		p = (struct partition *)(&buf[0x1be]);
		read_partitions(p);
		close(fd);
	}// end if
	return EXIT_SUCCESS;
}// end main

void read_partitions(struct partition *p){
    while(p->sys_type != 0 && p->drive == 0 ){
        printf("Part%d \t", part);
        printf(" %2x \t", p->sys_type);
		printf(" %2x \t", p->head);
		printf(" %8d \t", p->start_sector+estart);
		printf(" %8d\t", p->nr_sectors);
		printf(" %s\n", systems[p->sys_type][0]);

		if(p->sys_type == 5){
			estart += p->start_sector;
            lseek(fd, (long)(p->start_sector+sector_one_start)*512, SEEK_SET);
            if(sector_one_start == 0)
                sector_one_start = p->start_sector;
            read(fd,buf, 512);

            read_partitions((struct partition *)(&buf[0x1be]));
		}// end if
		p++;
    }// end while
}// end read_partitions

void fill_array(void){
	/*
	Portions of these system names have been taken from the open source code of
	fdisk at https://bitbucket.org/skypher/fdisk/
	It has been used under the GNU General Public License Copyright (C) 2006
*/
	systems[0x00][0] = "Empty";
	systems[0x01][0] = "FAT12";
	systems[0x02][0] = "XENIX root";
	systems[0x03][0] = "XENIX usr\0";
	systems[0x04][0] = "Small FAT16\0";
	systems[0x05][0] = "Extended\0";
	systems[0x06][0] = "FAT16\0";
	systems[0x07][0] = "HPFS/NTFS\0";
	systems[0x08][0] = "AIX\0";
	systems[0x09][0] = "AIX bootable\0";
	systems[0x0a][0] = "OS/2 boot mgr\0";
	systems[0x0b][0] = "FAT32\0";
	systems[0x0c][0] = "FAT32 LBA\0";
	systems[0x0e][0] = "FAT16 LBA\0";
	systems[0x0f][0] = "Extended LBA\0";
	systems[0x10][0] = "OPUS\0";
	systems[0x11][0] = "Hidden FAT12\0";
	systems[0x12][0] = "Compaq diag\0";
	systems[0x14][0] = "Hidd Sm FAT16\0";
	systems[0x16][0] = "Hidd FAT16\0";
	systems[0x17][0] = "Hidd HPFS/NTFS\0";
	systems[0x18][0] = "AST SmartSleep\0";
	systems[0x1b][0] = "Hidd FAT32\0";
	systems[0x1c][0] = "Hidd FAT32 LBA\0";
	systems[0x1e][0] = "Hidd FAT16 LBA\0";
	systems[0x24][0] = "NEC DOS\0";
	systems[0x27][0] = "Hidden NTFS WinRE\0";
	systems[0x39][0] = "Plan 9\0";
	systems[0x3c][0] = "PMagic recovery\0";
	systems[0x40][0] = "Venix 80286\0";
	systems[0x41][0] = "PPC PReP Boot\0";
	systems[0x42][0] = "SFS\0";
	systems[0x4d][0] = "QNX4.x\0";
	systems[0x4e][0] = "QNX4.x 2nd part\0";
	systems[0x4f][0] = "QNX4.x 3rd part\0";
	systems[0x50][0] = "OnTrack DM\0";
	systems[0x51][0] = "OnTrackDM6 Aux1\0";
	systems[0x52][0] = "CP/M\0";
	systems[0x53][0] = "OnTrackDM6 Aux3\0";
	systems[0x54][0] = "OnTrack DM6\0";
	systems[0x55][0] = "EZ Drive\0";
	systems[0x56][0] = "Golden Bow\0";
	systems[0x5c][0] = "Priam Edisk\0";
	systems[0x61][0] = "SpeedStor\0";
	systems[0x63][0] = "GNU HURD/SysV\0";
	systems[0x64][0] = "Netware 286\0";
	systems[0x65][0] = "Netware 386\0";
	systems[0x70][0] = "DiskSec MltBoot\0";
	systems[0x75][0] = "PC/IX\0";
	systems[0x80][0] = "Minix <1.4a\0";
	systems[0x81][0] = "Minix >1.4b\0";
	systems[0x82][0] = "Linux swap\0";
	systems[0x83][0] = "Linux\0";
	systems[0x84][0] = "OS/2 hidden C:\0";
	systems[0x85][0] = "Linux extended\0";
	systems[0x86][0] = "NTFS volume set\0";
	systems[0x87][0] = "NTFS volume set\0";
	systems[0x88][0] = "Linux plaintext\0";
	systems[0x8e][0] = "Linux LVM\0";
	systems[0x93][0] = "Amoeba\0";
	systems[0x94][0] = "Amoeba BBT\0";
	systems[0x9f][0] = "BSD/OS\0";
	systems[0xa0][0] = "Thinkpad hib\0";
	systems[0xa5][0] = "FreeBSD\0";
	systems[0xa6][0] = "OpenBSD\0";
	systems[0xa7][0] = "NeXTSTEP\0";
	systems[0xa8][0] = "Darwin UFS\0";
	systems[0xa9][0] = "NetBSD\0";
	systems[0xab][0] = "Darwin boot\0";
	systems[0xaf][0] = "HFS / HFS+\0";
	systems[0xb7][0] = "BSDI fs\0";
	systems[0xb8][0] = "BSDI swap\0";
	systems[0xbb][0] = "Boot Wizard Hid\0";
	systems[0xbe][0] = "Solaris boot\0";
	systems[0xbf][0] = "Solaris\0";
	systems[0xc1][0] = "DRDOS/2 FAT12\0";
	systems[0xc4][0] = "DRDOS/2 smFAT16\0";
	systems[0xc6][0] = "DRDOS/2 FAT16\0";
	systems[0xc7][0] = "Syrinx\0";
	systems[0xda][0] = "Non-FS data\0";
	systems[0xdb][0] = "CP/M / CTOS\0";
	systems[0xde][0] = "Dell Utility\0";
	systems[0xdf][0] = "BootIt\0";
	systems[0xe1][0] = "DOS access\0";
	systems[0xe3][0] = "DOS R/O\0";
	systems[0xe4][0] = "SpeedStor\0";
	systems[0xfb][0] = "VMware VMFS\0";
	systems[0xfc][0] = "VMware VMKCORE\0";
	systems[0xeb][0] = "BeOS fs\0";
	systems[0xee][0] = "GPT\0";
	systems[0xef][0] = "EFI FAT\0";
	systems[0xf0][0] = "Lnx/PA-RISC bt\0";
	systems[0xf1][0] = "SpeedStor\0";
	systems[0xf2][0] = "DOS secondary\0";
	systems[0xf4][0] = "SpeedStor\0";
	systems[0xfd][0] = "Lnx RAID auto\0";
	systems[0xfe][0] = "LANstep\0";
	systems[0xff][0] = "XENIX BBT\0";
	return;
}

