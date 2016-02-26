/* test_sam_led.c*/
// Nov 23,2013  Sam Zhou
// For LEDs Linux Device Driver Test APP


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


int main(int argc, char **argv){
	int fd;
	int count=0;
	char user_buff[2];
	
	if ((argc !=3)||(*argv[1]>'4')||(*argv[1]<'1')||(*argv[2]<'0' )||(*argv[2]>'1'))
	{
	printf("Usage:\n");
	printf("%s <1|2|3|4> <0|1>\n",argv[0]);
	printf("3 args are:%s %s %s,\n",argv[0],argv[1],argv[2]);
	printf("<1st.>:#of LED; <2nd.>:turn off/on\n");
	    exit (0);
	}


	fd = open("/dev/sam_led", O_RDWR);
	if (fd < 0) {
		perror("open device led");
		exit(1);
	}

	
	user_buff[0] = *argv[1];
	user_buff[1] = *argv[2];
	write(fd, user_buff,sizeof(int));
    	printf("-->hello!\n");
	close(fd);
	return 0;
}