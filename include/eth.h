#pragma once

#include "defs.h"
#include <stddef.h>

/* Opaque ptr type */
typedef struct eth_frame* eth_frame_t;

/* Ethernet status */
typedef enum eth_status {
	ETH_OKAY,
	ETH_BAD_SIZE,
	ETH_BAD_CRC32,
	ETH_OUT_OF_MEM,
	ETH_IO_ERR
} eth_status;

/**
 * Create an Ethernet frame
 *
 * @param src - source MAC address
 * @param dest - destination MAC address
 * @param data - data to transmit
 * @param d_amt - # of octets for the data
 * @param type - Ethernet frame type
 * @param status - return status of operation
 * @return the frame (NULL on error)
 */
eth_frame_t eth_create(const octet* src, const octet* dest, const octet* data, uint16_t d_amt,
					   const octet* type, eth_status* status);


/**
 * Write the Ethernet frame
 * @param frame - frame to write
 * @param fd - fd to write to
 */
void eth_write(eth_frame_t frame, int fd);

/**
 * Read an Ethernet frame
 * @param fd - fd to read from
 * @param status - return status of operation
 * @return the frame (NULL on error)
 */
eth_frame_t eth_read(int fd, eth_status* status);

/**
 * Destroy the Ethernet frame
 * @param frame - pointer to frame to free
 * @post *frame == NULL
 */
void eth_free(eth_frame_t* frame);

