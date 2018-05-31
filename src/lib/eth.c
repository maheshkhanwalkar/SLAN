/* eth.c - Ethernet frame support */
#include "eth.h"
#include "defs.h"
#include "msb.h"
#include "comm.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

/* Needed for crc32(...) */
/* TODO: eliminate this dependency */
#include <zlib.h>

/* Needed for endian conversions */
#include <endian.h>

/* Ethernet data section sizes */
#define ETH_MIN_SIZE 46
#define ETH_MAX_SIZE 1500

/* [Modified] Ethernet frame structure */
struct eth_frame {
	octet preamble[8];
	octet dest[6];
	octet src[6];
	octet type[2];
	octet* data;
	octet crc[4];
	uint16_t d_amt;
};

eth_frame_t eth_create(const octet* src, const octet* dest, const octet* data, uint16_t d_amt,
					   const octet* type, eth_status* status)
{
	if(!src || !dest || !data || !type || !status)
		return NULL;

	bool do_pad = false;
	uint16_t orig_amt = d_amt;

	if(d_amt < ETH_MIN_SIZE) {
		do_pad = true;
		d_amt = ETH_MIN_SIZE;
	}

	/* Data size is out of range */
	if(d_amt > ETH_MAX_SIZE) {
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

	memcpy(frame->data, data, orig_amt * sizeof(octet));

	/* Pad with zeros */
	if(do_pad) {
		memset(frame->data + orig_amt, 0, d_amt - orig_amt);
	}

	/* Compute CRC32 */
	/* FIXME: This might not be Ethernet standards compliant */
	uint32_t crc_raw = (uint32_t)crc32(0, data, (uInt)(d_amt * sizeof(octet)));
	msb_encode32(crc_raw, frame->crc);

	*status = ETH_OKAY;
	return frame;
}

void eth_free(eth_frame_t* frame)
{
	if(!frame || !*frame) {
		return;
	}

	free((*frame)->data);
	free(*frame);

	*frame = NULL;
}

void eth_write(eth_frame_t frame, int fd)
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

eth_frame_t eth_read(int fd, eth_status* status)
{
	/* Determine size */
	uint16_t d_amt;

	/* Determine data size */
	int ret = comm_force_read(fd, &d_amt, sizeof(uint16_t));

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

	ret = comm_force_read(fd, buf, size);

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
	frame = eth_create(src, dest, data, d_amt, type, &c_stat);

	if(!frame) {
		free(buf);
		*status = c_stat;
		return NULL;
	}

	*status = ETH_OKAY;
	free(buf);

	return frame;
}
