#include "comm.h"
#include <fcntl.h>

int comm_force_read(int fd, void* buf, size_t amt)
{
	ssize_t ret = read(fd, buf, amt);

	if(ret == -1) {
		return -1;
	}

	size_t pos = (size_t)ret;

	while((size_t)ret != amt) {
		ret = read(fd, (char*)buf+pos, amt - pos);

		if(ret == -1) {
			return -1;
		}

		pos += (size_t)ret;
	}

	return 0;
}

int comm_toggle_blocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);

	if(flags == -1) {
		return -1;
	}

	/* Toggle O_NONBLOCK */
	if((flags | O_NONBLOCK) == flags) {
		flags &= ~O_NONBLOCK;
	} else {
		flags |= O_NONBLOCK;
	}

	return fcntl(fd, F_SETFL, flags);
}
