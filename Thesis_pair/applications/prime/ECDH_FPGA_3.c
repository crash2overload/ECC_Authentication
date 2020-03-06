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

	double seconds;
	struct timespec tstart={0,0}, tend={0,0};

	if ((dd = open_physical (dd)) == -1)
      return (-1);
    
    Curve_params_t *curve = &bn256;
	
	//Create Users Alice, Bob, Carol
	Node_t Alice, Bob, Carol;
	Curve_to_Mont(curve, dd);
	
	init_node(curve, &Alice);
	init_node(curve, &Bob);
	init_node(curve, &Carol);
		
	clock_gettime(CLOCK_MONOTONIC, &tstart);
	
	//Random Keys = Round 1
	ecc_Make_Key(curve, &Alice, dd);
	ecc_Make_Key(curve, &Bob, dd);
	ecc_Make_Key(curve, &Carol, dd);
	
	//Round 2
	Point_t X_B, X_C1, X_C2;
	init_Point(curve, &X_B);
	init_Point(curve, &X_C1);
	init_Point(curve, &X_C2);
	memcpy (X_B.x, Alice.Point.x, curve->prec/8);
	memcpy (X_B.y, Alice.Point.y, curve->prec/8);
	memcpy (X_B.z, Alice.Point.z, curve->prec/8);
	
	memcpy (X_C1.x, Alice.Point.x, curve->prec/8);
	memcpy (X_C1.y, Alice.Point.y, curve->prec/8);
	memcpy (X_C1.z, Alice.Point.z, curve->prec/8);
	
	memcpy (X_C2.x, Bob.Point.x, curve->prec/8);
	memcpy (X_C2.y, Bob.Point.y, curve->prec/8);
	memcpy (X_C2.z, Bob.Point.z, curve->prec/8);
	
	ecc_Point_Multi(curve, &Bob.privateKey[0], &X_B, dd);
	ecc_Point_Multi(curve, &Carol.privateKey[0], &X_C1, dd);
	ecc_Point_Multi(curve, &Carol.privateKey[0], &X_C2, dd);
	
	//Round 3
	ecc_Point_Multi(curve, &Bob.privateKey[0], &X_C1, dd);
	ecc_Point_Multi(curve, &Carol.privateKey[0], &X_B, dd);
	ecc_Point_Multi(curve, &Alice.privateKey[0], &X_C2, dd);
	
	memcpy (Bob.Point.x, X_C2.x, curve->prec/8);
	memcpy (Bob.Point.y, X_C2.y, curve->prec/8);
	memcpy (Bob.Point.z, X_C2.z, curve->prec/8);
	
	memcpy (Alice.Point.x, X_C1.x, curve->prec/8);
	memcpy (Alice.Point.y, X_C1.y, curve->prec/8);
	memcpy (Alice.Point.z, X_C1.z, curve->prec/8);
	
	memcpy (Carol.Point.x, X_B.x, curve->prec/8);
	memcpy (Carol.Point.y, X_B.y, curve->prec/8);
	memcpy (Carol.Point.z, X_B.z, curve->prec/8);
			
	jacobian_to_affine(curve, &Alice, dd);
	Mont_to_Affine(&Alice.Point, curve, dd);
	
	jacobian_to_affine(curve, &Bob, dd);
	Mont_to_Affine(&Bob.Point, curve, dd);
	
	jacobian_to_affine(curve, &Carol, dd);
	Mont_to_Affine(&Carol.Point, curve, dd);
	
	clock_gettime(CLOCK_MONOTONIC, &tend);
	
	if(0 == memcmp(Alice.Point.x, Bob.Point.x, curve->prec/8)) {
		if(0 == memcmp(Alice.Point.x, Carol.Point.x, curve->prec/8))
			printf("Shared Secret equal\n");
	}
	else
		printf("Shared Secret failed\n");
	
	
	seconds = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
	if (seconds*1000000.0 > 1000.0)
		printf("3-Party-ECDH took about %.5f ms\n\n", seconds*1000.0);
	else 
		printf("3-Party-ECDH took about %.5f us\n\n", seconds*1000000.0);	

	free_node(&Alice);
	free_node(&Bob);
	free_node(&Carol);
	
	free_Point(&X_B);
	free_Point(&X_C1);
	free_Point(&X_C2);	
	close_physical (dd);   // close /dev/cryptocore
	return 0;
}
