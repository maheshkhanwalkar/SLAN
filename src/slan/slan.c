#include <stdio.h>

/**
 * slan.c
 *
 * This program allows for the complete configuration of SLAN
 * e.g. it handles spawning, talking with, and shutting down the
 * various Ethernet simulation daemons.
 */

int main(int argc, const char* argv[])
{
	/* No args */
	if(argc < 2) {
		fprintf(stderr, "slan: error: no arguments provided\n");
		return -1;
	}

	/* TODO handle argument parsing */
	(void)argv;
	return 0;
}
