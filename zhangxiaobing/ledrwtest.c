#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

//#define led_in_out 0 //0 as input ---read, 1 as output--write

int main(void)
{
	int val, count;
	int fd, minor;
	unsigned char key_val[4];
	
	int i=0;

	if((fd = open("/dev/led_zxb", O_RDWR)) < 0)
	{
		perror("can't open device led\n");
		exit(1);
	}
	
	printf("input minor to decide read or write:\n");//1 as write
	scanf("%d",&minor);
	switch(minor){
	case 1:	
		printf("start to write\n");
		scanf("%d",&val);
		write(fd, &val, sizeof(val));
		break;

	case 0:
		printf("start to read\n");
		while(1){
			read(fd, key_val, sizeof(key_val));
			if(!key_val[0] || !key_val[1] || 
				!key_val[2] || !key_val[3]){

				printf("04%d are pressed:%d %d %d %d\n", 						i++, key_val[0], key_val[1], 						key_val[2], key_val[3]);
			}
		}
		
		break;
	default:
		break;
	}
	close(fd);
}
