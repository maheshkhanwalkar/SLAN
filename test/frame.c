
#include "defs.h"
#include "eth.h"

#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
	/* Random MACs */
	octet src[6] = {0x7a, 0xac, 0x7d, 0x0c, 0xe4, 0x56};
	octet dest[6] = {0xcb, 0x25, 0xc6, 0xac, 0xda, 0x8b};

	const char* s_data = "Hello world. This is some text to make the frame "
					  "long enough to work without error";

	octet* data = (octet*)s_data;
	uint16_t d_amt = (uint16_t)(strlen(s_data) / sizeof(octet));

	octet type[2] = {0x00, 0x00};

	eth_status status;
	eth_frame_t orig = Eth_Create(src, dest, data, d_amt, type, &status);

	int fd = open("test.txt", O_WRONLY | O_CREAT, 0644);
	Eth_Write(orig, fd);
	close(fd);

	fd = open("test.txt", O_RDONLY);
	eth_frame_t dup = Eth_Read(fd, &status);

	Eth_Free(dup);
	Eth_Free(orig);
}

