#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main()
{
	const char *payload = "hello from userspace";
	char read_buf[128];
	ssize_t bytes_written;
	ssize_t bytes_read;
	int dev = open("/dev/mydevice", O_RDWR);

	if (dev == -1) {
		perror("open(/dev/mydevice)");
		printf("Opening was not possible!\n");
		printf("errno=%d\n", errno);
		return -1;
	}

	bytes_written = write(dev, payload, strlen(payload));
	if (bytes_written == -1) {
		perror("write(/dev/mydevice)");
		close(dev);
		return -2;
	}

	if (close(dev) == -1) {
		perror("close(/dev/mydevice)");
		return -3;
	}

	dev = open("/dev/mydevice", O_RDONLY);
	if (dev == -1) {
		perror("open(/dev/mydevice) for read");
		return -3;
	}

	bytes_read = read(dev, read_buf, sizeof(read_buf) - 1);
	if (bytes_read == -1) {
		perror("read(/dev/mydevice)");
		close(dev);
		return -4;
	}

	read_buf[bytes_read] = '\0';

	printf("open: OK\n");
	printf("write: %zd bytes\n", bytes_written);
	printf("read:  %zd bytes\n", bytes_read);
	printf("data:  '%s'\n", read_buf);

	close(dev);
	return 0;
}