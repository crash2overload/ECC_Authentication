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

#include "../../include/cryptocore_ioctl_header.h"
#include "../../include/mont_util.h"
#include "../../include/jacobian.h"
#include "../../include/ecc_util.h"
#include "../../include/ecc_signatur.h"
#include "../../include/curves.h"


/* Prototypes for functions used to access physical memory addresses */
int open_physical (int);
void close_physical (int);

int main(void)
{
	int dd = -1;
	int ret_val;

	__u32 trng_val = 0;
	__u32 i = 0;
	
	double seconds;
	struct timespec tstart={0,0}, tend={0,0};

	if ((dd = open_physical (dd)) == -1)
      return (-1);

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
	
	Curve_params_t curve = {
	{ 0xFFFFFFFF, 0x00000001, 0x00000000, 0x00000000, 
	  0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},
	{ 0xFFFFFFFF, 0x00000001, 0x00000000, 0x00000000,
	  0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFC},
	{ 0x5AC635D8, 0xAA3A93E7, 0xB3EBBD55, 0x769886BC,
	  0x651D06B0, 0xCC53B0F6, 0x3BCE3C3E, 0x27D2604B},
	{  },
	{ 0x6B17D1F2, 0xE12C4247, 0xF8BCE6E5, 0x63A440F2,
	  0x77037D81, 0x2DEB33A0, 0xF4A13945, 0xD898C296},
	{ 0x4FE342E2, 0xFE1A7F9B, 0x8EE7EB4A, 0x7C0F9E16,
	  0x2BCE3357, 0x6B315ECE, 0xCBB64068, 0x37BF51F5},
	{ 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
	  0xBCE6FAAD, 0xA7179E84, 0xF3B9CAC2, 0xFC632551}
	};
	
	Node_t Alice, Bob;
	Curve_to_Mont(&curve, dd);
	
	memcpy (Alice.poin.x, secp256r1.G_x, sizeof(Alice.poin.x));
	memcpy (Alice.poin.y, secp256r1.G_y, sizeof(Alice.poin.x));
	affine_to_jacobian(&curve, &Alice, dd);
	
	clock_gettime(CLOCK_MONOTONIC, &tstart);
	
	ecc_Make_Key(&curve, &Alice, dd);
	
	uint8_t msg[512], s[32], r[32];
	memset(s, 0, sizeof(s));
	memset(r, 0, sizeof(r));
	//memset(msg, 1, sizeof(msg));
			
	ecc_get_Signatur(&curve, &Alice, msg, sizeof(msg), r, s, sizeof(s), dd);
	
	printf("s: 0x");
	for (i = 0; i < sizeof(s); i++)
		printf("%x", s[i]);
	printf("\n\n");
	printf("r: 0x");
	for (i = 0; i < sizeof(r); i++)
		printf("%x", r[i]);
	printf("\n\n");
	
		
	ecc_verfi_Signatur(&curve, &Alice, msg, sizeof(msg), r, s, sizeof(s), dd);		
	
	
	jacobian_to_affine(&curve, &Alice, dd);
	Mont_to_Affine(&Alice.poin, &curve, dd);
	
	clock_gettime(CLOCK_MONOTONIC, &tend);
	
	
	
	
	//printf("X = : 0x");
	//for(i=0; i < 256/32; i++){
		//printf("%02x", Alice.poin.x[i]);
	//}
	//printf("\n\n");
	
	//printf("Y = : 0x");
	//for(i=0; i < 256/32; i++){
		//printf("%08x", Alice.poin.y[i]);
	//}
	//printf("\n\n");
	
	//printf("Z = : 0x");
	//for(i=0; i < 256/32; i++){
		//printf("%08x", Alice.poin.z[i]);
	//}
	//printf("\n\n");
	
	//printf("X = : 0x");
	//for(i=0; i < 256/32; i++){
		//printf("%02x", Bob.poin.x[i]);
	//}
	//printf("\n\n");
	
	//printf("Y = : 0x");
	//for(i=0; i < 256/32; i++){
		//printf("%08x", Bob.poin.y[i]);
	//}
	//printf("\n\n");
	
	//printf("Z = : 0x");
	//for(i=0; i < 256/32; i++){
		//printf("%08x", Bob.poin.z[i]);
	//}
	//printf("\n\n");
	
	
	seconds = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
	if (seconds*1000000.0 > 1000.0)
		printf("MontMult 512 took about %.5f ms\n\n", seconds*1000.0);
	else 
		printf("MontMult 512 took about %.5f us\n\n", seconds*1000000.0);	

		
	close_physical (dd);   // close /dev/cryptocore
	return 0;
}

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
