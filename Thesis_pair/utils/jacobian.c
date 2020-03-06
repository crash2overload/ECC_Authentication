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

//#include "../include/cryptocore_ioctl_header.h"
//#include "../include/node.h"
#include "../include/jacobian.h"

int affine_to_jacobian(Curve_params_t *curve, Node_t *node, int dd)
{
	int ret_val = 0;
	Core_CMD_t core_cmd = { curve->prec, 1,	0,
	MONTR,{ },{ },{ },{ },{ },{ },{ }};
	
	//Calculate R and save as G_y
	memcpy (core_cmd.P, curve->n, curve->prec/8);
	
	//Write n to P
	ret_val = ioctl(dd, IOCTL_MWMAC_CLEAR, &core_cmd);
	core_cmd.dest = MWMAC_RAM_P;
	ret_val = ioctl(dd, IOCTL_MWMAC_WRITE_RAM, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	core_cmd.dest = MWMAC_RAM_B1;
	core_cmd.src = MWMAC_RAM_P1;	
	ret_val = ioctl(dd, IOCTL_MWMAC_MONTR, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	core_cmd.dest = MWMAC_RAM_B;
	ret_val = ioctl(dd, IOCTL_MWMAC_READ_RAM, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	memcpy(node->Point.z, core_cmd.B, curve->prec/8);
	
	return ret_val;
}

int affine_to_jacobian2(Curve_params_t *curve, Point_t *Point, int dd)
{
	int ret_val = 0;
	Core_CMD_t core_cmd = { curve->prec, 1,	0,
	MONTR,{ },{ },{ },{ },{ },{ },{ }};
	
	//Calculate R and save as G_y
	memcpy (core_cmd.P, curve->n, curve->prec/8);
	
	//Write n to P
	ret_val = ioctl(dd, IOCTL_MWMAC_CLEAR, &core_cmd);
	core_cmd.dest = MWMAC_RAM_P;
	ret_val = ioctl(dd, IOCTL_MWMAC_WRITE_RAM, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	core_cmd.dest = MWMAC_RAM_B1;
	core_cmd.src = MWMAC_RAM_P1;	
	ret_val = ioctl(dd, IOCTL_MWMAC_MONTR, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	core_cmd.dest = MWMAC_RAM_B;
	ret_val = ioctl(dd, IOCTL_MWMAC_READ_RAM, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	memcpy(Point->z, core_cmd.B, curve->prec/8);
	
	return ret_val;
}

int jacobian_to_affine(Curve_params_t *curve, Node_t *node, int dd)
{
	int ret_val = 0;
	int i;
	Core_CMD_t core_cmd = { curve->prec, 1,	0,
	MONTR,{ },{ },{ },{ },{ },{ },{ }};
	
	memcpy (core_cmd.P, curve->n, curve->prec/8);
	memcpy (core_cmd.E, curve->n, curve->prec/8);
	core_cmd.E[(core_cmd.prec/32)-1] = core_cmd.E[(core_cmd.prec/32)-1] - 2;
	
	memcpy(&core_cmd.X[0], node->Point.x, curve->prec/8);
	memcpy(&core_cmd.X[16], node->Point.y, curve->prec/8);
	memcpy(&core_cmd.X[32], node->Point.z, curve->prec/8);
	
	ret_val = ioctl(dd, IOCTL_MWMAC_DEJACOBIAN, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	memcpy(node->Point.x, &core_cmd.X[0], curve->prec/8);
	memcpy(node->Point.y, &core_cmd.X[16], curve->prec/8);
		
	return ret_val;
}

int jacobian_to_affine2(Curve_params_t *curve, Point_t *P, int dd)
{
	int ret_val = 0;
	int i;
	Core_CMD_t core_cmd = { curve->prec, 1,	0,
	MONTR,{ },{ },{ },{ },{ },{ },{ }};
	
	memcpy (core_cmd.P, curve->n, curve->prec/8);
	memcpy (core_cmd.E, curve->n, curve->prec/8);
	core_cmd.E[(core_cmd.prec/32)-1] = core_cmd.E[(core_cmd.prec/32)-1] - 2;
	
	memcpy(&core_cmd.X[0], P->x, curve->prec/8);
	memcpy(&core_cmd.X[16], P->y, curve->prec/8);
	memcpy(&core_cmd.X[32], P->z, curve->prec/8);
	
	ret_val = ioctl(dd, IOCTL_MWMAC_DEJACOBIAN, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	memcpy(P->x, &core_cmd.X[0], curve->prec/8);
	memcpy(P->y, &core_cmd.X[16], curve->prec/8);
		
	return ret_val;
}
