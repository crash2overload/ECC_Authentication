#include <asm-generic/fcntl.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <inttypes.h>

#include "../include/cryptocore_util.h"
#include "../include/cryptocore_ioctl_header.h"


// Open /dev/cryptocore, if not already done, to give access to physical addresses
int open_physical (int dd)
{
   if (dd == -1)
      if ((dd = open( "/dev/cryptocore", (O_RDWR | O_SYNC))) == -1)
      {
         printf ("ERROR: could not open \"/dev/cryptocore\"...\n");
         return (-1);
      }
   return dd;
}

// Close /dev/cryptocore to give access to physical addresses
void close_physical (int dd)
{
   close (dd);
}

int init_trng (int dd)
{
	int ret_val = 0;
	__u32 trng_val = 0;
	// Stop TRNG and clear FIFO
	trng_val = 0x00000010;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_CMD, &trng_val);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

	usleep(10);

// Configure Feedback Control Polynomial
	trng_val = 0x0003ffff;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_CTR, &trng_val);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

// Configure Stabilisation Time
	trng_val = 0x00000050;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_TSTAB, &trng_val);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

// Configure Sample Time
	trng_val = 0x00000006;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_TSAMPLE, &trng_val);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

// Start TRNG
	trng_val = 0x00000001;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_CMD, &trng_val);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

	usleep(10);
	return ret_val;
}

int make_random_Element(__u32 *value, int Precision)
{
	int ret_val = 0;
	__u32 trng_val = 0;
	__u32 i = 0;
	int dd = -1;
	if ((dd = open_physical (dd)) == -1)
      return (-1);
		
	init_trng(dd);
	
	// Read random b from TRNG FIRO
	i = 0;
	while (i < Precision/32) {
		ret_val = ioctl(dd, IOCTL_READ_TRNG_FIFO, &trng_val);
		if(ret_val == 0) {
			value[i] = trng_val;
			i++;
		} else if (ret_val == -EAGAIN) {
			printf("TRNG FIFO empty\n");
		} else {
			printf("Error occured\n");
		}
	}
	close_physical(dd);
	return ret_val;
}
