#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main()
{
	int dev = open("/dev/mydevice", O_RDONLY);
	if (dev == -1) {
		perror("open(/dev/mydevice)");
		printf("Opening was not possible!\n");
		printf("errno=%d\n", errno);
		return -1;
	}
	printf("Opening was successfull!\n");
	close(dev);
	return 0;
}