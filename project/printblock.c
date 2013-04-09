#include <stdio.h>
#include <stdlib.h>

int tstbit(char *buf, int bit){
	int i,j;
	i = bit /8;
	j = bit %8;
	return (buf[i] & (1 << j));
}

int main(int argc, const char *argv[])
{
	int fd,i,j;
	char buff[1024];
	open(fd, argv[1]);
	if(fd==0)
		exit(-1);
	lseek(fd, (int)argv[2]*1024,SEEK_SET);
	read(fd, buff, 1024);
	

	for (i = 0; i < 1024*8; i++) {
		printf("%x ",tstbit(buff,i) );
	}
	return 0;
}
