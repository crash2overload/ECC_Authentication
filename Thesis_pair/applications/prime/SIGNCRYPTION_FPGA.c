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

#ifndef CRYPTO
#define CRYPTO
#include "../../include/crypto.h"
#endif

int main(void)
{
	int dd = -1;
	__u32 i = 0;

	double seconds;
	struct timespec tstart={0,0}, tend={0,0};

	if ((dd = open_physical (dd)) == -1)
      return (-1);

	Curve_params_t *curve = &bn256;

   	Node_t Alice, Bob;
	Curve_to_Mont(curve, dd);
	
	init_node(curve, &Alice);
	init_node(curve, &Bob);	

	clock_gettime(CLOCK_MONOTONIC, &tstart);
	__u32 r[8], s[8], msg;
	size_t msg_len;
	msg = 0x0;
	msg_len = sizeof(msg);

	ecc_Make_Key(curve, &Alice, dd);
	ecc_Make_Key(curve, &Bob, dd);
	
	make_random_Key(&Alice.sharedKey[0], 256, dd);
	
	Signcryption(curve, &Alice, &Bob.Point, &msg, msg_len, &r, &s, dd);
	printf("r = 0x");
	for(i = 0; i < curve->prec/32; i++)
		printf("%08x", r[i]);
	printf("\n");
	printf("s = 0x");
	for(i = 0; i < curve->prec/32; i++)
		printf("%08x", s[i]);
	printf("\n");
	printf("c = 0x%08x \n", msg);
	Unsigncryption(curve, &Bob, &Alice.Point, &msg, msg_len, &r, &s, dd);


	clock_gettime(CLOCK_MONOTONIC, &tend);


	seconds = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
	if (seconds*1000000.0 > 1000.0)
		printf("Signcryption took about %.5f ms\n\n", seconds*1000.0);
	else
		printf("Signcryption took about %.5f us\n\n", seconds*1000000.0);


	close_physical (dd);   // close /dev/cryptocore
	return 0;
}
