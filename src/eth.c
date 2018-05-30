/* eth.c - Ethernet frame support */
#include "eth.h"
#include "defs.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

/* Needed for crc32(...) */
/* TODO: eliminate this dependency */
#include <zlib.h>

/* Needed for endian conversions */
#include <endian.h>

#define ETH_MIN_SIZE 46
#define ETH_MAX_SIZE 1500

/* (Modified) Ethernet frame structure */
struct eth_frame {
	octet preamble[8];
	octet dest[6];
	octet src[6];
	octet type[2];
	octet* data;
	uint16_t d_amt;
	octet crc[4];
};

/**
 * Encode src into dest using Most Significant Byte (MSB)
 * @param src - source 32-bit int
 * @param dest - dest octet buffer
 */
static inline void msb_encode32(uint32_t src, octet* dest)
{
	uint32_t good = htobe32(src);
	memcpy(dest, &good, sizeof(uint32_t));
}

/**
 * Decode src into dest using MSB
 * @param src - source octets
 * @param dest - destination int
 */
static inline void msb_decode32(const octet* src, uint32_t* dest)
{
	*dest = be32toh(*(uint32_t*)src);
}

/**
 * Encode src into dest using MSB
 * @param src - source 64-bit int
 * @param dest - dest octet buffer
 */
static inline void msb_encode64(uint64_t src, octet* dest)
{
	uint64_t good = htobe64(src);
	memcpy(dest, &good, sizeof(uint64_t));
}

eth_frame_t Eth_Create(const octet* src, const octet* dest, const octet* data, uint16_t d_amt,
					   const octet* type, eth_status* status)
{
	if(!src || !dest || !data || !type)
		return NULL;

	/* Data size is out of range */
	if(d_amt < ETH_MIN_SIZE || d_amt > ETH_MAX_SIZE) {
		*status = ETH_BAD_SIZE;
		return NULL;
	}

	eth_frame_t frame = malloc(sizeof(struct eth_frame));

	if(!frame) {
		*status = ETH_OUT_OF_MEM;
		return NULL;
	}

	/* Create defensive copy of data */
	frame->data = malloc(d_amt * sizeof(octet));
	frame->d_amt = d_amt;

	if(!frame->data) {
		free(frame);
		*status = ETH_OUT_OF_MEM;
		return NULL;
	}

	/* 10101010 -> AA */
	/* 10101011 -> AB */
	uint64_t pattern = 0xAAAAAAAAAAAAAAAB;

	/* Setup frame */
	msb_encode64(pattern, frame->preamble);
	memcpy(frame->dest, dest, sizeof(frame->dest));
	memcpy(frame->src, src, sizeof(frame->src));
	memcpy(frame->type, type, sizeof(frame->type));
	memcpy(frame->data, data, d_amt * sizeof(octet));

	/* Compute CRC32 */
	/* FIXME: This might not be Ethernet standards compliant */
	uint32_t crc_raw = (uint32_t)crc32(0, data, (uInt)(d_amt * sizeof(octet)));
	msb_encode32(crc_raw, frame->crc);

	*status = ETH_OKAY;
	return frame;
}

void Eth_Free(eth_frame_t* frame)
{
	if(!frame || !*frame) {
		return;
	}

	free((*frame)->data);
	free(*frame);

	*frame = NULL;
}

void Eth_Write(eth_frame_t frame, int fd)
{
	if(!frame) {
		return;
	}

	/* Convert to network byte order */
	uint16_t msb = htobe16(frame->d_amt);

	/* Write the frame */

	/* NOTE: This deviates from actual Ethernet procedure,
	 * since this method writes the data (payload) size
	 * before the actual frame, whereas standard Ethernet
	 * does not write any such field. */

	/* This deviation is done for simplicity reasons, since
	 * there is no inter-packet gap, so it becomes hard to tell
	 * where exactly a packet ends (there is no guarantee that
	 * the Ethernet device writes out only one packet) */

	write(fd, &msb, sizeof(frame->d_amt));
	write(fd, frame->preamble, sizeof(frame->preamble));
	write(fd, frame->dest, sizeof(frame->dest));
	write(fd, frame->src, sizeof(frame->src));
	write(fd, frame->type, sizeof(frame->type));
	write(fd, frame->data, sizeof(octet) * frame->d_amt);
	write(fd, frame->crc, sizeof(frame->crc));
}

/**
 * Force 'amt' bytes to be read into buf
 * @param fd - fd to read from
 * @param buf - buffer to store data in
 * @param amt - amt to read
 * @return 0 on success, -1 on failure
 */
static int force_read(int fd, void* buf, size_t amt)
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

		pos += ret;
	}

	return 0;
}


eth_frame_t Eth_Read(int fd, eth_status* status)
{
	/* Determine size */
	uint16_t d_amt;

	/* Determine data size */
	int ret = force_read(fd, &d_amt, sizeof(uint16_t));

	if(ret == -1) {
		*status = ETH_IO_ERR;
		return NULL;
	}

	/* MSB to Host Conversion */
	d_amt = be16toh(d_amt);

	/* Bad size (data size *must* be [46, 1500] octets) */
	if(d_amt > ETH_MAX_SIZE || d_amt < ETH_MIN_SIZE) {
		*status = ETH_BAD_SIZE;
		return NULL;
	}

	eth_frame_t frame;

	/* Compute frame size */
	size_t size = sizeof(frame->preamble) + sizeof(frame->dest)
			+ sizeof(frame->src) + sizeof(frame->type) + d_amt + sizeof(frame->crc);

	uint8_t* buf = malloc(size);

	if(!buf) {
		*status = ETH_OUT_OF_MEM;
		return NULL;
	}

	ret = force_read(fd, buf, size);

	if(ret == -1) {
		free(buf);

		*status = ETH_IO_ERR;
		return NULL;
	}

	uint8_t* preamble = buf;

	uint8_t* dest = preamble + sizeof(frame->preamble);
	uint8_t* src = dest + sizeof(frame->dest);
	uint8_t* type = src + sizeof(frame->src);

	uint8_t* data = type + sizeof(frame->type);
	uint8_t* crc_raw = data + sizeof(octet) * d_amt;

	/* Compute CRC32 */
	/* FIXME: This might not be Ethernet standards compliant */
	uint32_t check, crc;
	check = (uint32_t)crc32(0, data, sizeof(octet) * d_amt);
	msb_decode32(crc_raw, &crc);

	/* Bad frame */
	if(check != crc) {
		free(buf);
		*status = ETH_BAD_CRC32;
		return NULL;
	}

	/* Create the frame */
	eth_status c_stat;
	frame = Eth_Create(src, dest, data, d_amt, type, &c_stat);

	if(!frame) {
		free(buf);
		*status = c_stat;
		return NULL;
	}

	*status = ETH_OKAY;
	free(buf);

	return frame;
}
