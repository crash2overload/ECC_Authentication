#include <asm-generic/fcntl.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef CRYPTO
#define CRYPTO
#include "../../include/crypto.h"
#endif

int main(void)
{
	int dd = -1, i;
	
		
	double seconds;
	struct timespec tstart={0,0}, tend={0,0};

	if ((dd = open_physical (dd)) == -1)
      return (-1);
	
	Curve_params_t *curve = &bn192;
	
	Node_t Alice, Bob, G;
	Curve_to_Mont(curve, dd);
	
	init_node(curve, &Alice);
	init_node(curve, &Bob);
	init_node(curve, &G);
	
	memcpy (G.Point.x, curve->G_x, curve->prec/8);
	memcpy (G.Point.y, curve->G_y, curve->prec/8);
	affine_to_jacobian(curve, &G, dd);
	
	clock_gettime(CLOCK_MONOTONIC, &tstart);
	
	ecc_Make_Key(curve, &Alice, dd);
	ecc_Make_Key(curve, &Bob, dd);

	ecc_Point_Multi(curve, Alice.privateKey, &Bob.Point, dd);
	ecc_Point_Multi(curve, Bob.privateKey, &Alice.Point, dd);
	
	jacobian_to_affine(curve, &Alice, dd);
	Mont_to_Affine(&Alice.Point, curve, dd);
	
	jacobian_to_affine(curve, &Bob, dd);
	Mont_to_Affine(&Bob.Point, curve, dd);
	
	clock_gettime(CLOCK_MONOTONIC, &tend);
	
	printf("Shared Secret = : 0x");
	for(i=0; i < curve->prec/32; i++){
		printf("%08x", Alice.Point.x[i]);
	}
	printf("\n\n");
	
	printf("Shared Secret = : 0x");
	for(i=0; i < curve->prec/32; i++){
		printf("%08x", Bob.Point.x[i]);
	}
	printf("\n\n");
	
	if (0 == memcmp(Bob.Point.x, Alice.Point.x, curve->prec/8))
		printf("Shared Secret equal.\n");
	else
		printf("Shared Secret failed.\n");
	
	
	seconds = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
	if (seconds*1000000.0 > 1000.0)
		printf("ECDH took about %.5f ms\n\n", seconds*1000.0);
	else 
		printf("ECDH took about %.5f us\n\n", seconds*1000000.0);	

	free_node(&Alice);	
	free_node(&Bob);
	close_physical (dd);   // close /dev/cryptocore
	return 0;
}


