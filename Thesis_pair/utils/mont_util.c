#include <asm-generic/fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>

#include "../include/field.h"
#include "../include/mont_util.h"
#include "../include/cryptocore_ioctl_header.h"
#include "../include/cryptocore_util.h"
#include "../include/curves.h"

int EFp_to_proj(EFp *P){
	
	int ret_val = 0;
	int dd = -1;
		
	Core_CMD_t cmd_test = { 192, 1,	0, MONTGOMERIZE,
	{ },{ },{ },{ },{ },{ },{ }	};
	
	memcpy (cmd_test.P, &bn192.n[0], 192/8);
	memcpy (&cmd_test.B[0], P->x.x0, 192/8);
	memcpy (&cmd_test.B[16], P->y.x0, 192/8);
	if ((dd = open_physical (dd)) == -1)
      return (-1);	
	ret_val = ioctl(dd, IOCTL_MWMAC_MONTGOMERIZE, &cmd_test);
	close_physical(dd);
	memcpy (P->x.x0, &cmd_test.B[0], 192/8);
	memcpy (P->y.x0, &cmd_test.B[16], 192/8);
	memcpy (P->z.x0, &cmd_test.B[32], 192/8);
	
	
	return ret_val;
}

int EFp_to_affine(EFp *P){
	int ret_val = 0;
	int i;
	Core_CMD_t core_cmd = { 192, 1,	0,
	MONTR,{ },{ },{ },{ },{ },{ },{ }};
	
	memcpy (core_cmd.P, &bn192.n[0], 192/8);
	memcpy (core_cmd.E, &bn192.n[0], 192/8);
	core_cmd.E[(core_cmd.prec/32)-1] = core_cmd.E[(core_cmd.prec/32)-1] - 2;
	
	memcpy(&core_cmd.X[0], P->x.x0, 192/8);
	memcpy(&core_cmd.X[16], P->y.x0, 192/8);
	memcpy(&core_cmd.X[32], P->z.x0, 192/8);
	
	int dd = -1;
	if ((dd = open_physical (dd)) == -1)
      return (-1);
	
	ret_val = ioctl(dd, IOCTL_MWMAC_DEJACOBIAN, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	close_physical(dd);	

	memcpy(P->x.x0, &core_cmd.X[0], 192/8);		
	memcpy(P->y.x0, &core_cmd.X[16], 192/8);
	return ret_val;
	
}


/*
int Mont_Mult(Curve_params_t *curve, __u32 *value1, __u32 *value2, int dd){
	int ret_val = 0;
	
	Core_CMD_t cmd_test = { curve->prec, 1,	0, MONTR2,
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
	memcpy(value1, cmd_test.B, cmd_test.prec/8);
	
	return ret_val;
}

int Mod_Add(Curve_params_t *curve, __u32 *value1, __u32 *value2, int dd){
	int ret_val = 0;
	
	Core_CMD_t cmd_test = { curve->prec, 1,	0, MODADD,
	{ },{ },{ },{ },{ },{ },{ }	};
	
	memcpy (cmd_test.P, curve->order, cmd_test.prec/8);
	memcpy (cmd_test.TS, value1, cmd_test.prec/8);
	memcpy (cmd_test.TC, value2, cmd_test.prec/8);
		
	//Write n to P
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
	memcpy(value1, cmd_test.B, cmd_test.prec/8);
			
	return ret_val;
}

*/
