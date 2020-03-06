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
#include <blake2.h>

#include "../include/cryptocore_ioctl_header.h"
#include "../include/ecc_util.h"
#include "../include/ecc_signatur.h"
#include "../include/sha3.h"

void swapper(__u32 *hash, int hash_len)
{
		int i = 0;
		uint8_t helper[4];
		
		__u32 zweiter = 0x0;
		for (i = 0; i < hash_len/4; i++) {
			memcpy (&helper[0], &hash[i], 4);
			zweiter = helper[3] << 0 | helper[2] << 8 | helper[1] << 16 | helper[0] << 24;
			memcpy (&hash[i], &zweiter, 4);
		}
}

int ecc_get_Signatur(Curve_params_t * curve, Node_t *node, const void *msg, 
						int msg_len, void *r, void *s, int hash_len, int dd)
{
	int ret_val, i;
	Node_t temp;
	
	Core_CMD_t core_cmd = { 256, 1,	0,
	MONTMULT,{0x0},{0x0},{0x0},{0x0},{0x0},{0x0},{0x0}};
	
	memcpy (temp.poin.x, curve->G_x, sizeof(temp.poin.x));
	memcpy (temp.poin.y, curve->G_y, sizeof(temp.poin.x));
	affine_to_jacobian(curve, &temp, dd);
	
	//Make a emphermal Key
	ecc_Make_Key(curve, &temp, dd);
	
	printf("Temp Private Key: 0x");
	for(i=0; i < core_cmd.prec/32; i++){
		printf("%08x", temp.privateKey[i]);
	}
	printf("\n\n");
	
	jacobian_to_affine(curve, &temp, dd);
	Mont_to_Affine(&temp.poin, curve, dd);
	
	printf("\n\n%i\n\n", hash_len);
	
	printf("X Temp: 0x");
	for(i=0; i < core_cmd.prec/32; i++){
		printf("%08x", temp.poin.x[i]);
	}
	printf("\n\n");
	//Calculate the HASH
	
	memset(s, 0, hash_len);
	memset(&core_cmd.B[16], 0, 64);
	uint8_t hash[32];
	
	blake2s_state state;
	blake2s_init(&state, 32);
	//blake2s_init_key(&state, hash_len, &temp.privateKey[0], 32);
	blake2s_update(&state, msg, msg_len);
	blake2s_final(&state, &hash[0], hash_len);
	
	printf("hash: 0x");
	for(i=0; i < 32; i++){
		printf("%02x", hash[i]);
	}
	
	//sha3(msg, msg_len, s, hash_len);
	//Copy everything to struct
	//swapper(&core_cmd.B[16], hash_len);
	memcpy(&core_cmd.B[16], &hash[0], hash_len);
	
	memcpy (core_cmd.E, curve->n, sizeof(core_cmd.P));
	core_cmd.E[(core_cmd.prec/32)-1] = core_cmd.E[(core_cmd.prec/32)-1] - 2;
	memcpy(&core_cmd.B[0], node->privateKey, core_cmd.prec/8);

	memcpy(&core_cmd.B[32], &temp.poin.x, core_cmd.prec/8);
	memcpy(&core_cmd.B[48], &temp.privateKey, core_cmd.prec/8);
	memcpy(core_cmd.P, curve->n, sizeof(core_cmd.P));
	memcpy(r, &temp.poin.x, core_cmd.prec/8);
	
	swapper(&core_cmd.B[16], hash_len);
	
	//Get the Party started
	ret_val = ioctl(dd, IOCTL_MWMAC_SIGNATURE, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	printf("\nh : 0x");
	for(i=0; i < 256/32; i++){
		printf("%08x", core_cmd.B[16+i]);
	}
	printf("\n\n");	
	memcpy(s, &core_cmd.B[0], core_cmd.prec/8);
    	
	return ret_val;
}

void get_Hash(const void *msg, int msg_len, void *hash, int hash_len)
{
	int i;
	memset(hash, 0, hash_len);
	sha3(msg, msg_len, hash, hash_len);
	swapper(hash, hash_len);
}

int ecc_verfi_Signatur(Curve_params_t * curve, Node_t *node, const void *msg, 
						int msg_len, void *r, void *s, int hash_len, int dd)
{
	int ret_val, i;
	Node_t temp;
	
	Core_CMD_t core_cmd = { 256, 1,	0,
	MONTMULT,{0x0},{0x0},{0x0},{0x0},{0x0},{0x0},{0x0}};
	
	memcpy (temp.poin.x, curve->G_x, sizeof(temp.poin.x));
	memcpy (temp.poin.y, curve->G_y, sizeof(temp.poin.x));
	affine_to_jacobian(curve, &temp, dd);
	
	
		
	//Calculate the HASH
	__u32 hash[8];
	memset(hash, 0, hash_len);
	sha3(msg, msg_len, hash, hash_len);
	
	//Copy everything to struct
	swapper(&hash[0], hash_len);
	memcpy(core_cmd.P, curve->n, sizeof(core_cmd.P));
	memcpy (core_cmd.E, curve->n, sizeof(core_cmd.P));
	core_cmd.E[(core_cmd.prec/32)-1] = core_cmd.E[(core_cmd.prec/32)-1] - 2;
	
	memcpy(&core_cmd.B[16], s, core_cmd.prec/8);
	memcpy(&core_cmd.B[32], r, core_cmd.prec/8);
	memcpy(&core_cmd.B[48], &hash, core_cmd.prec/8);
	
	//Get the Party started
	ret_val = ioctl(dd, IOCTL_MWMAC_VERIFICATION, &core_cmd);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	ecc_Point_Multi(curve, &core_cmd.B[0], &node->poin, dd);
	ecc_Point_Multi(curve, &core_cmd.B[16], &temp.poin, dd);
	
	//ecc_Point_Add(curve, &temp.poin, &node->poin, dd);
	
	jacobian_to_affine(curve, &temp.poin, dd);
	Mont_to_Affine(&temp.poin, curve, dd);
	
	jacobian_to_affine(curve, &node->poin, dd);
	Mont_to_Affine(&node->poin, curve, dd);
		
	printf("X = : 0x");
	for(i=0; i < 256/32; i++){
		printf("%02x", node->poin.x[i]);
	}
	printf("\n\n");
	
	printf("Y = : 0x");
	for(i=0; i < 256/32; i++){
		printf("%08x", node->poin.y[i]);
	}
	printf("\n\n");
	
	printf("X = : 0x");
	for(i=0; i < 256/32; i++){
		printf("%02x", temp.poin.x[i]);
	}
	printf("\n\n");
	
	printf("Y = : 0x");
	for(i=0; i < 256/32; i++){
		printf("%08x", temp.poin.y[i]);
	}
	printf("\n\n");
		    	
	return ret_val;
}
