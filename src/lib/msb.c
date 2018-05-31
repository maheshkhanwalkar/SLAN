#include "msb.h"

#include <endian.h>
#include <string.h>

void msb_encode32(uint32_t src, octet* dest)
{
	uint32_t good = htobe32(src);
	memcpy(dest, &good, sizeof(uint32_t));
}

void msb_decode32(const octet* src, uint32_t* dest)
{
	*dest = be32toh(*(const uint32_t*)src);
}

void msb_encode64(uint64_t src, octet* dest)
{
	uint64_t good = htobe64(src);
	memcpy(dest, &good, sizeof(uint64_t));
}
