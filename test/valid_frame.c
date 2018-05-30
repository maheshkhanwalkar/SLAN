
#include "defs.h"
#include "eth.h"

#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <assert.h>
#include <endian.h>

/* Hack to break the "abstraction" */
struct eth_frame {
	octet preamble[8];
	octet dest[6];
	octet src[6];
	octet type[2];
	octet* data;
	uint16_t d_amt;
	octet crc[4];
};

void test_create()
{
	/* Randomly generated Ethernet (MAC) addresses*/
	octet src[6] = {0x7a, 0xac, 0x7d, 0x0c, 0xe4, 0x56};
	octet dest[6] = {0xcb, 0x25, 0xc6, 0xac, 0xda, 0x8b};

	const char* s_data = "Hello world. This is some text to make the frame "
						 "long enough to work without error";

	octet* data = (octet*)s_data;
	uint16_t d_amt = (uint16_t)(strlen(s_data) / sizeof(octet));

	octet type[2] = {0x00, 0x00};

	eth_status status;
	eth_frame_t frame = Eth_Create(src, dest, data, d_amt, type, &status);

	assert(frame != NULL);

	assert(*(uint64_t*)frame->preamble == htobe64(0xAAAAAAAAAAAAAAAB));
	assert(memcmp(dest, frame->dest, sizeof(dest)) == 0);
	assert(memcmp(src, frame->src, sizeof(src)) == 0);
	assert(memcmp(type, frame->type, sizeof(type)) == 0);
	assert(frame->d_amt == d_amt);
	assert(strncmp(s_data, (const char*)frame->data, d_amt) == 0);

	Eth_Free(&frame);
	assert(frame == NULL);
}


int main(void)
{
	test_create();
	return 0;
}

