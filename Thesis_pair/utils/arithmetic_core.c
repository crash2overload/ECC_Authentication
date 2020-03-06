#include <asm/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/field.h"
#include "../include/arithmetic_core.h"
#include "../include/cryptocore_ioctl_header.h"
#include "../include/curves.h"

int Fp_mult_core(__u32 *res, __u32 *value1, __u32 *value2){
	int dd = -1;
	if ((dd = open_physical (dd)) == -1)
      return (-1);
      
    int ret_val = 0;
    Curve_params_t *curve = &bn192;
	
	Core_CMD_t cmd_test = { curve->prec, 1, 0, MONTR2,
	{ },{ },{ },{ },{ },{ },{ }	};
	
	memcpy (cmd_test.P, curve->order, curve->prec/8);
		
	//Write n to P
	ret_val = ioctl(dd, IOCTL_MWMAC_CLEAR, &cmd_test);
	cmd_test.dest = MWMAC_RAM_P;
	ret_val = ioctl(dd, IOCTL_MWMAC_WRITE_RAM, &cmd_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	//Calculate R2 saved in A		
	cmd_test.dest = MWMAC_RAM_B1;
	cmd_test.src = MWMAC_RAM_P1;	
	ret_val = ioctl(dd, IOCTL_MWMAC_MONTR, &cmd_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}	
	
	cmd_test.src = MWMAC_RAM_B1;
	cmd_test.dest = MWMAC_RAM_A1;
	cmd_test.cmd = COPYH2V;
	ret_val = ioctl(dd, IOCTL_MWMAC_COPY, &cmd_test);	
	
	//Calculate Value1*R
	memcpy(cmd_test.B , value1, cmd_test.prec/8);
	cmd_test.dest = MWMAC_RAM_B;
	ret_val = ioctl(dd, IOCTL_MWMAC_WRITE_RAM, &cmd_test);
	
	cmd_test.cmd = MONTMULT;
	cmd_test.src = MWMAC_RAM_B1;
	cmd_test.dest = MWMAC_RAM_A1;
	ret_val = ioctl(dd, IOCTL_MWMAC_MONTMULT, &cmd_test);
	
	cmd_test.src = MWMAC_RAM_B1;
	cmd_test.dest = MWMAC_RAM_A2;
	cmd_test.cmd = COPYH2V;
	ret_val = ioctl(dd, IOCTL_MWMAC_COPY, &cmd_test);
	
	//Calculate value2*R
	memcpy(cmd_test.B , value2, cmd_test.prec/8);
	cmd_test.dest = MWMAC_RAM_B;
	ret_val = ioctl(dd, IOCTL_MWMAC_WRITE_RAM, &cmd_test);
	
	cmd_test.cmd = MONTMULT;
	cmd_test.src = MWMAC_RAM_B1;
	cmd_test.dest = MWMAC_RAM_A1;
	ret_val = ioctl(dd, IOCTL_MWMAC_MONTMULT, &cmd_test);
	
	cmd_test.cmd = MONTMULT;
	cmd_test.src = MWMAC_RAM_B1;
	cmd_test.dest = MWMAC_RAM_A2;
	ret_val = ioctl(dd, IOCTL_MWMAC_MONTMULT, &cmd_test);
	
	cmd_test.cmd = MONTMULT1;
	cmd_test.src = MWMAC_RAM_B1;
	ret_val = ioctl(dd, IOCTL_MWMAC_MONTMULT, &cmd_test);
	
	cmd_test.dest = MWMAC_RAM_B;
	ret_val = ioctl(dd, IOCTL_MWMAC_READ_RAM, &cmd_test);
	memcpy(res, cmd_test.B, cmd_test.prec/8);
	
	close_physical (dd);
	
	return ret_val;
}

int Fp_inv_core(__u32 *element)
{
	int ret_val = 0;
	int i;
	Core_CMD_t core_cmd = { 192, 1,	0,
	INVERT_ELEM,{ },{ },{ },{ },{ },{ },{ }};
	
	memcpy(core_cmd.P, &bn192.order[0], 192/8);
	memcpy(core_cmd.E, &bn192.order[0], 192/8);
	core_cmd.E[(core_cmd.prec/32)-1] = core_cmd.E[(core_cmd.prec/32)-1] - 2;
	
	memcpy(&core_cmd.X[0], element, 192/8);
	
	int dd = -1;
	if ((dd = open_physical (dd)) == -1)
      return (-1);	
	
	ret_val = ioctl(dd, IOCTL_MWMAC_INVERT_ELEM, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	close_physical(dd);
	
	memcpy(element, &core_cmd.B[0], 192/8);
			
	return ret_val;
}

int Fp_add_core(__u32 *res, __u32 *value1, __u32 *value2)
{
	int ret_val = 0;
	
	Core_CMD_t cmd_test = { 192, 1,	0, MODADD,
	{ },{ },{ },{ },{ },{ },{ }	};
	
	memcpy (cmd_test.P, &bn192.order[0], cmd_test.prec/8);
	memcpy (cmd_test.TS, value1, cmd_test.prec/8);
	memcpy (cmd_test.TC, value2, cmd_test.prec/8);
		
	//Write n to P
	int dd = -1;
	if ((dd = open_physical (dd)) == -1)
      return (-1);	
	ret_val = ioctl(dd, IOCTL_MWMAC_CLEAR, &cmd_test);
	cmd_test.dest = MWMAC_RAM_P;
	ret_val = ioctl(dd, IOCTL_MWMAC_WRITE_RAM, &cmd_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	//Write a to TS
	cmd_test.dest = MWMAC_RAM_TS;
	ret_val = ioctl(dd, IOCTL_MWMAC_WRITE_RAM, &cmd_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	//Write b to TC
	cmd_test.dest = MWMAC_RAM_TC;
	ret_val = ioctl(dd, IOCTL_MWMAC_WRITE_RAM, &cmd_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	cmd_test.src = MWMAC_RAM_TS1;
	cmd_test.cmd = MODADD;
	ret_val = ioctl(dd, IOCTL_MWMAC_MODADD, &cmd_test);	
	
	cmd_test.dest = MWMAC_RAM_B;
	ret_val = ioctl(dd, IOCTL_MWMAC_READ_RAM, &cmd_test);
	memcpy(res, cmd_test.B, cmd_test.prec/8);
	
	close_physical(dd);		
	return ret_val;

}

int Fp_sub_core(__u32 *res, __u32 *value1, __u32 *value2)
{
	int ret_val = 0;
	
	Core_CMD_t cmd_test = { 192, 1,	0, MODADD,
	{ },{ },{ },{ },{ },{ },{ }	};
	
	memcpy (cmd_test.P, &bn192.order[0], cmd_test.prec/8);
	memcpy (cmd_test.TS, value1, cmd_test.prec/8);
	memcpy (cmd_test.TC, value2, cmd_test.prec/8);
		
	//Write n to P
	int dd = -1;
	if ((dd = open_physical (dd)) == -1)
      return (-1);	
	ret_val = ioctl(dd, IOCTL_MWMAC_CLEAR, &cmd_test);
	cmd_test.dest = MWMAC_RAM_P;
	ret_val = ioctl(dd, IOCTL_MWMAC_WRITE_RAM, &cmd_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	//Write a to TS
	cmd_test.dest = MWMAC_RAM_TS;
	ret_val = ioctl(dd, IOCTL_MWMAC_WRITE_RAM, &cmd_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	//Write b to TC
	cmd_test.dest = MWMAC_RAM_TC;
	ret_val = ioctl(dd, IOCTL_MWMAC_WRITE_RAM, &cmd_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	cmd_test.src = MWMAC_RAM_TS1;
	cmd_test.cmd = MODADD;
	ret_val = ioctl(dd, IOCTL_MWMAC_MODSUB, &cmd_test);	
	
	cmd_test.dest = MWMAC_RAM_B;
	ret_val = ioctl(dd, IOCTL_MWMAC_READ_RAM, &cmd_test);
	memcpy(res, cmd_test.B, cmd_test.prec/8);
	
	close_physical(dd);		
	return ret_val;

}

int Fp_exp_core(__u32 *res, __u32 *element, __u32 *skalar){
	
	int ret_val = 0;
	int i;
	Core_CMD_t core_cmd = { 192, 1,	0,
	INVERT_ELEM,{ },{ },{ },{ },{ },{ },{ }};
	
	memcpy(core_cmd.P, &bn192.order[0], core_cmd.prec/8);
	memcpy(core_cmd.E, skalar, core_cmd.prec/8);
	
	memcpy(&core_cmd.X[0], element, core_cmd.prec/8);
	
	int dd = -1;
	if ((dd = open_physical (dd)) == -1)
      return (-1);	
	
	ret_val = ioctl(dd, IOCTL_MWMAC_MONTEXP, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	close_physical(dd);
	
	memcpy(res, &core_cmd.B[0], core_cmd.prec/8);
			
	return ret_val;
}

int EFp_add_core(EFp *res, EFp *P, EFp *Q){
	
	int ret_val = 0;
	
	Core_CMD_t core_cmd = { 192, 1,	0,
	ECC_POINT_ADD,{0},{0},{0},{0},{0},{0},{0}};
	core_cmd.X[0] = 0x2;
	//memcpy(&core_cmd.X[80], curve->a, curve->prec/8);
	//memcpy(&core_cmd.X[96], curve->b, curve->prec/8);	
	memcpy(&core_cmd.E[0], P->x.x0, 192/8);
	memcpy(&core_cmd.E[16], P->y.x0, 192/8);
	memcpy(&core_cmd.E[32], P->z.x0, 192/8);
	memcpy(&core_cmd.E[48], Q->x.x0, 192/8);
	memcpy(&core_cmd.E[64], Q->y.x0, 192/8);
	memcpy(&core_cmd.E[80], Q->z.x0, 192/8);
	
		
	memcpy(core_cmd.P, &bn192.n[0], 192/8);
	
	int dd = -1;
	if ((dd = open_physical (dd)) == -1)
      return (-1);
	
	ret_val = ioctl(dd, IOCTL_MWMAC_ECC_POINT_ADD, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	close_physical(dd);	
	memcpy(res->x.x0, &core_cmd.E[0], 192/8);
	memcpy(res->y.x0, &core_cmd.E[16], 192/8);
	memcpy(res->z.x0, &core_cmd.E[32], 192/8);
	
	return ret_val;
	
	
}

int EFp_mul_core(__u32 *skalar, EFp *P)
{
	int dd = -1;
	if ((dd = open_physical (dd)) == -1)
      return (-1);
      
	int ret_val = 0;
	int i = 0;
	Core_CMD_t core_cmd = { 192, 1,	0,
	ECC_POINT_ADD,{0x0},{0x0},{0x0},{0x0},{0x0},{0x0},{0x0}};
	core_cmd.X[0] = 0x2;
	core_cmd.X[1] = 0x3;
	core_cmd.X[2] = 0x4;
	core_cmd.X[3] = 0x8;
	memcpy(&core_cmd.E[48], P->x.x0, 192/8);
	memcpy(&core_cmd.E[64], P->y.x0, 192/8);
	memcpy(&core_cmd.E[80], P->z.x0, 192/8);
	
	memcpy(&core_cmd.A[0], &skalar[0], 192/8);
	
	memcpy(core_cmd.P, &bn192.n[0], 192/8);
	
	ret_val = ioctl(dd, IOCTL_MWMAC_ECC_POINT_MULTIPLICATION, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

	memcpy(P->x.x0, &core_cmd.E[0], 192/8);
	memcpy(P->y.x0, &core_cmd.E[16], 192/8);
	memcpy(P->z.x0, &core_cmd.E[32], 192/8);
	close_physical(dd);
	return ret_val;	
}
