#pragma once

#include <stddef.h>
#include <unistd.h>

/**
* Force 'amt' bytes to be read into buf
* @param fd - fd to read from
* @param buf - buffer to store data in
* @param amt - amt to read
* @return 0 on success, -1 on failure
*/
int comm_force_read(int fd, void* buf, size_t amt);

/**
 * Attempt to toggle fd's blocking state, e.g.
 * non-blocking -> blocking or blocking -> non-blocking
 * @param fd - fd to modify
 * @return -1 on error
 */
int comm_toggle_blocking(int fd);
