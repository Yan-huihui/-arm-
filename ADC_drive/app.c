#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char *argv[])
{
	float temper = 0.0;
	int adc = 0;
	int ret = -1;
	int fd = open("/dev/temper", O_RDWR);
	if(fd < 0)
	{
		perror("open led fail");
		exit(-1);
	}
	
	while(1)
	{
		adc = 0;
		printf("read\n");
		ret = read(fd, &adc, 1);
		printf("user : %d\n", adc);
		temper = 0.1903 * adc - 30.7565;
		printf("temper : %f",temper);
		sleep(1);
	}

	close(fd);

	return 0;
}
