#include <asm-generic/fcntl.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "../../include/cryptocore_ioctl_header.h"
#include "../../include/cryptocore_util.h"
#include "../../include/node.h"
#include "../../include/mont_util.h"
#include "../../include/jacobian.h"
#include "../../include/ecc_util.h"


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
	
	clock_gettime(CLOCK_MONOTONIC, &tstart);
	
	init_node(curve, &Alice);
	init_node(curve, &Bob);
	
	ecc_Make_Key(curve, &Alice, dd);
	ecc_Make_Key(curve, &Bob, dd);
	
	__u32 hash[8]={0};
	__u32 r[8]={0};
	
	make_random_Key(&hash[0], curve->prec, dd);

	jacobian_to_affine(curve, &Bob, dd);
	Mont_to_Affine(&Bob.Point, curve, dd);

	memcpy(&r[0], &Bob.Point.x[0], curve->prec/8);
	
	Mont_Mult(curve, &Bob.Point.x[0], &Alice.privateKey[0], dd);
	
	Mod_Add(curve, &Bob.Point.x[0], &hash[0], dd);
	
	invert_element(curve, &Bob.privateKey[0], dd);
		
	Mont_Mult(curve, &Bob.Point.x[0], &Bob.privateKey[0], dd);
	
	printf("s = : 0x");
	for(i=0; i < curve->prec/32; i++){
		printf("%08x", Bob.Point.x[i]);
	}
	printf("\n\n");
	
	printf("r = : 0x");
	for(i=0; i < curve->prec/32; i++){
		printf("%08x", r[i]);
	}
	printf("\n\n");
	
	/**********  Verifikation ***********************/
			
	
	invert_element(curve, &Bob.Point.x[0], dd);
	
	//u1 in hash
	Mont_Mult(curve, &hash[0], &Bob.Point.x[0], dd);
	//u2
	__u32 r1[8];
	memcpy(&r1, &r, curve->prec/8);
	Mont_Mult(curve, &r1[0], &Bob.Point.x[0], dd);
	 //u1 * G
	memcpy (Bob.Point.x, curve->G_x, curve->prec/8);
	memcpy (Bob.Point.y, curve->G_y, curve->prec/8);
	affine_to_jacobian(curve, &Bob, dd);
	
	ecc_Point_Multi(curve, &hash[0], &Bob.Point, dd);
	ecc_Point_Multi(curve, &r1[0], &Alice.Point, dd);
	
	ecc_Point_Add(curve, &Alice.Point, &Bob.Point, dd);
	
	jacobian_to_affine(curve, &Alice, dd);
	Mont_to_Affine(&Alice.Point, curve, dd);
	
	jacobian_to_affine(curve, &Bob, dd);
	Mont_to_Affine(&Bob.Point, curve, dd);
	
	clock_gettime(CLOCK_MONOTONIC, &tend);
	
	if (0 == memcmp(Alice.Point.x, r, curve->prec/8))
		printf("Signature verified\n");
	else
		printf("Verification failed\n");
	
	seconds = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
	if (seconds*1000000.0 > 1000.0)
		printf("ECDSA took about %.5f ms\n\n", seconds*1000.0);
	else 
		printf("ECDSA 512 took about %.5f us\n\n", seconds*1000000.0);	

	free_node(&Alice);
	free_node(&Bob);
		
	close_physical (dd);   // close /dev/cryptocore
	return 0;
}
