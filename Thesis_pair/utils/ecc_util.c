#include <asm-generic/fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>

#include "../include/ecc_util.h"


int ecc_Point_Add(Curve_params_t * curve, Point_t *PointP, Point_t *PointQ, int dd)
{
	int ret_val = 0;
	int i = 0;
	Core_CMD_t core_cmd = { curve->prec, 1,	0,
	ECC_POINT_ADD,{0},{0},{0},{0},{0},{0},{0}};
	core_cmd.X[0] = 0x2;
	memcpy(&core_cmd.X[80], curve->a, curve->prec/8);
	memcpy(&core_cmd.X[96], curve->b, curve->prec/8);	
	memcpy(&core_cmd.E[0], PointP->x, curve->prec/8);
	memcpy(&core_cmd.E[16], PointP->y, curve->prec/8);
	memcpy(&core_cmd.E[32], PointP->z, curve->prec/8);
	memcpy(&core_cmd.E[48], PointQ->x, curve->prec/8);
	memcpy(&core_cmd.E[64], PointQ->y, curve->prec/8);
	memcpy(&core_cmd.E[80], PointQ->z, curve->prec/8);
	
		
	memcpy(core_cmd.P, curve->n, curve->prec/8);
	
	//core_cmd.prec = 256;
	ret_val = ioctl(dd, IOCTL_MWMAC_ECC_POINT_ADD, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
		
	memcpy(PointP->x, &core_cmd.E[0], curve->prec/8);
	memcpy(PointP->y, &core_cmd.E[16], curve->prec/8);
	memcpy(PointP->z, &core_cmd.E[32], curve->prec/8);
	
	return ret_val;
}

int ecc_Point_Double(Curve_params_t * curve, Point_t *PointP, int dd)
{
	int ret_val = 0;
	int i = 0;
	Core_CMD_t core_cmd = { curve->prec, 1,	0,
	ECC_POINT_DOUBLE,{0},{0},{0},{0},{0},{0},{0}};
	
	core_cmd.X[0] = 0x2;
	core_cmd.X[1] = 0x3;
	core_cmd.X[2] = 0x4;
	core_cmd.X[3] = 0x8;
	memcpy(&core_cmd.X[80], curve->a, curve->prec/8);
	memcpy(&core_cmd.X[96], curve->b, curve->prec/8);	
	memcpy(&core_cmd.E[0], PointP->x, curve->prec/8);
	memcpy(&core_cmd.E[16], PointP->y, curve->prec/8);
	memcpy(&core_cmd.E[32], PointP->z, curve->prec/8);
	
	memcpy(core_cmd.P, curve->n, curve->prec/8);
	
	ret_val = ioctl(dd, IOCTL_MWMAC_ECC_POINT_DOUBLE, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	memcpy(PointP->x, &core_cmd.E[0], curve->prec/8);
	memcpy(PointP->y, &core_cmd.E[16], curve->prec/8);
	memcpy(PointP->z, &core_cmd.E[32], curve->prec/8); 
	
	return ret_val;	
}

int ecc_Point_Multi(Curve_params_t * curve, __u32 *skalar, Point_t *PointP, int dd)
{
	int ret_val = 0;
	int i = 0;
	Core_CMD_t core_cmd = { curve->prec, 1,	0,
	ECC_POINT_ADD,{0x0},{0x0},{0x0},{0x0},{0x0},{0x0},{0x0}};
	core_cmd.X[0] = 0x2;
	core_cmd.X[1] = 0x3;
	core_cmd.X[2] = 0x4;
	core_cmd.X[3] = 0x8;
	memcpy(&core_cmd.X[80], curve->a, curve->prec/8);
	memcpy(&core_cmd.X[96], curve->b, curve->prec/8);	
	memcpy(&core_cmd.E[48], PointP->x, curve->prec/8);
	memcpy(&core_cmd.E[64], PointP->y, curve->prec/8);
	memcpy(&core_cmd.E[80], PointP->z, curve->prec/8);
	
	memcpy(&core_cmd.A[0], &skalar[0], curve->prec/8);
	
	memcpy(core_cmd.P, curve->n, curve->prec/8);
	
	ret_val = ioctl(dd, IOCTL_MWMAC_ECC_POINT_MULTIPLICATION, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

	memcpy(PointP->x, &core_cmd.E[0], curve->prec/8);
	memcpy(PointP->y, &core_cmd.E[16], curve->prec/8);
	memcpy(PointP->z, &core_cmd.E[32], curve->prec/8);
	
	return ret_val;	
}

int ecc_Make_Key(Curve_params_t * curve, Node_t *node, int dd)
{
	int ret_val = 0;
	__u32 trng_val = 0;
	__u32 i = 0;
	Core_CMD_t core_cmd = { curve->prec, 1,	0,
	ECC_POINT_MULTI,{},{},{},{},{ },{ },{ }};
	
	init_trng(dd);
	
	// Read random b from TRNG FIRO
	i = 0;
	while (i < core_cmd.prec/32) {
		ret_val = ioctl(dd, IOCTL_READ_TRNG_FIFO, &trng_val);
		if(ret_val == 0) {
			node->privateKey[i] = trng_val;
			i++;
		} else if (ret_val == -EAGAIN) {
			printf("TRNG FIFO empty\n");
		} else {
			printf("Error occured\n");
		}
	}
	
	// Make Public/Private Key starting from G
	memcpy (node->Point.x, curve->G_x, curve->prec/8);
	memcpy (node->Point.y, curve->G_y, curve->prec/8);
	affine_to_jacobian(curve, node, dd);
	
	ecc_Point_Multi(curve, node->privateKey, &node->Point, dd);

	return ret_val;
}

void print_Point(Node_t *node)
{
	__u32 i = 0;

	printf("\n X - Koordinate: ");
	for (i = 0; i < 256/32; i++)
		printf("%08x ", node->Point.x[i]);

	printf("\n Y - Koordinate: ");
	for (i = 0; i < 256/32; i++)
		printf("%08x ", node->Point.y[i]);

	printf("\n Z - Koordinate: ");
	for (i = 0; i < 256/32; i++)
		printf("%08x ", node->Point.z[i]);
		
	printf("\n");

}
