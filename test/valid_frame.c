
#include "defs.h"
#include "eth.h"

#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <assert.h>
#include <endian.h>
#include <stdlib.h>

/* Ethernet data section sizes */
#define ETH_MIN_SIZE 46
#define ETH_MAX_SIZE 1500

/* Hack to break the "abstraction" */
struct eth_frame {
	octet preamble[8];
	octet dest[6];
	octet src[6];
	octet type[2];
	octet* data;
	octet crc[4];
	uint16_t d_amt;
};

/* Tests Eth_Create() */
static void test_create_good()
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
	assert(status == ETH_OKAY);

	assert(*(uint64_t*)frame->preamble == htobe64(0xAAAAAAAAAAAAAAAB));
	assert(memcmp(dest, frame->dest, sizeof(dest)) == 0);
	assert(memcmp(src, frame->src, sizeof(src)) == 0);
	assert(memcmp(type, frame->type, sizeof(type)) == 0);
	assert(frame->d_amt == d_amt);
	assert(strncmp(s_data, (const char*)frame->data, d_amt) == 0);

	Eth_Free(&frame);
	assert(frame == NULL);
}

static void test_create_bad()
{
	eth_frame_t res = Eth_Create(NULL, NULL, NULL, 0, NULL, NULL);
	assert(res == NULL);

	octet* buf = malloc(sizeof(octet));
	assert(buf != NULL);

	eth_status status;
	res = Eth_Create(buf, buf, buf, 50000, buf, &status);

	assert(res == NULL);
	assert(status == ETH_BAD_SIZE);

	free(buf);
}

static void test_create_padding()
{
	/* Randomly generated Ethernet (MAC) addresses*/
	octet src[6] = {0x7a, 0xac, 0x7d, 0x0c, 0xe4, 0x56};
	octet dest[6] = {0xcb, 0x25, 0xc6, 0xac, 0xda, 0x8b};

	const char* s_data = "Small data";

	octet* data = (octet*)s_data;
	uint16_t d_amt = (uint16_t)(strlen(s_data) / sizeof(octet));

	octet type[2] = {0x00, 0x00};

	eth_status status;
	eth_frame_t frame = Eth_Create(src, dest, data, d_amt, type, &status);

	assert(frame != NULL);
	assert(status == ETH_OKAY);

	assert(*(uint64_t*)frame->preamble == htobe64(0xAAAAAAAAAAAAAAAB));
	assert(memcmp(dest, frame->dest, sizeof(dest)) == 0);
	assert(memcmp(src, frame->src, sizeof(src)) == 0);
	assert(memcmp(type, frame->type, sizeof(type)) == 0);
	assert(frame->d_amt == ETH_MIN_SIZE);
	assert(strncmp(s_data, (const char*)frame->data, d_amt) == 0);

	for(uint16_t i = 0; i < ETH_MIN_SIZE - d_amt; i++) {
		assert(*(frame->data + d_amt + i) == 0);
	}

	Eth_Free(&frame);
	assert(frame == NULL);
}

static void test_create()
{
	test_create_good();
	test_create_bad();
	test_create_padding();
}

int main(void)
{
	test_create();
	return 0;
}

