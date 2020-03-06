#include <asm-generic/fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <blake2.h>
#include <stdlib.h>

#ifndef CRYPTO
#define CRYPTO
#include "../include/crypto.h"
#endif

#include "../include/aes.h"

int Signcryption(Curve_params_t *curve, Node_t *node, Point_t *publicKey, void *msg, size_t msg_len, void *r, void *s, int dd)
{
	int ret_val = 0;
	__u32 i = 0;
	__u32 tempKey[16] = {0x0};
	uint8_t m_key[32], e_key[32];
	uint8_t mess[4]={0};
	Point_t pubKey;
	init_Point(curve, &pubKey); 

	memcpy(&pubKey.x[0], publicKey->x, curve->prec/8);
	memcpy(&pubKey.y[0], publicKey->y, curve->prec/8);
	memcpy(&pubKey.z[0], publicKey->z, curve->prec/8);

	ecc_Point_Multi(curve, node->sharedKey, &pubKey, dd);
	jacobian_to_affine2(curve, &pubKey, dd);
	Mont_to_Affine(&pubKey, curve, dd);
	memcpy(&m_key[0], &pubKey.x[0], curve->prec/16);
	memcpy(&e_key[0], &pubKey.x[4], curve->prec/16);

	uint8_t *hash;
	hash = (uint8_t*)malloc(curve->prec/8);
	memset(hash, 0, curve->prec/8);
	
	blake2s_state state;
	
	blake2s_init_key(&state, curve->prec/8, &m_key[0], curve->prec/16);
	blake2s_update(&state, msg, msg_len);
	blake2s_final(&state, hash, curve->prec/8);
	
	__u32 re[16];

	memcpy(&re[0], hash, curve->prec/8);
	for(i=0; i < curve->prec/32; i++)
		re[i] = htonl(re[i]);
		
	memcpy(r, &re[0], curve->prec/8);

	printf("Nachricht plain: \n");
	for(i = 0; i < 4; i++)
		printf("%02x ", mess[i]);
	printf("\n");
	
	uint8_t iv=0x0;
	struct AES_ctx ctx;
	AES_init_ctx(&ctx, &e_key[0]);
	AES_init_ctx_iv(&ctx, &e_key[0], &iv);
	AES_CTR_xcrypt_buffer(&ctx, &mess[0], 4);

	printf("\n");
	printf("Nachricht verschlüsselt: \n");
	for(i = 0; i < 4; i++)
		printf("%02x ", mess[i]);
	printf("\n");
	
	Mod_Add(curve, &re[0], node->privateKey, dd);
	invert_element(curve, &re[0], dd);
	Mont_Mult(curve, &re[0], node->sharedKey, dd);
	
		
	memcpy(s, &re[0], curve->prec/8);
	__u32 hilfs;
	hilfs = ((mess[0]<<0) | (mess[1]<<8)| (mess[2]<<16) | (mess[3]<<24));
	memcpy(msg, &hilfs, 4);
	
	free(hash);
	free_Point(&pubKey);
	
	return ret_val;
}

int Unsigncryption (Curve_params_t *curve, Node_t *node, Point_t *publicKey, void *msg, size_t msg_len, void *r, void *s, int dd)
{
	int ret_val = 0;
	__u32 i = 0;
	__u32 tempKey[16] = {0x0};
	uint8_t m_key[32], e_key[32];
	uint8_t mess[4];
	Point_t R;
	init_Point(curve, &R);
	
	__u32 tempS[8], tempR[8];
	
	memcpy(&tempS[0], s, curve->prec/8);
	memcpy(&tempR[0], r, curve->prec/8);
	memcpy(&mess[0], msg, 4);
	
	Mont_Mult(curve, &tempS[0], node->privateKey, dd);
	
	memcpy (&R.x[0], curve->G_x, curve->prec/8);
	memcpy (&R.y[0], curve->G_y, curve->prec/8);
	affine_to_jacobian2(curve, &R, dd);
	
	ecc_Point_Multi(curve, &tempR[0], &R, dd);
	ecc_Point_Add(curve, &R, publicKey, dd);
	ecc_Point_Multi(curve, &tempS[0], &R, dd);
	
	jacobian_to_affine2(curve, &R, dd);
	Mont_to_Affine(&R, curve, dd);
	
	memcpy(&m_key[0], &R.x[0], curve->prec/16);
	memcpy(&e_key[0], &R.x[4], curve->prec/16);
	
	printf("Nachricht verschlüsselt: \n");
	for(i = 0; i < 4; i++)
		printf("%02x ", mess[i]);
	printf("\n");
	
	uint8_t iv=0x0;
	struct AES_ctx ctx;
	AES_init_ctx(&ctx, &e_key[0]);
	AES_init_ctx_iv(&ctx, &e_key[0], &iv);
	AES_CTR_xcrypt_buffer(&ctx, &mess[0], 4);
	
	printf("Nachricht entschlüsselt: \n");
	for(i = 0; i < 4; i++)
		printf("%02x ", mess[i]);
	printf("\n");
	
	uint8_t *hash;
	hash = (uint8_t*)malloc(curve->prec/8);
	memset(hash, 0, curve->prec/8);

	blake2s_state state;
	blake2s_init_key(&state, curve->prec/8, &m_key[0], curve->prec/16);
	blake2s_update(&state, mess, sizeof(mess));
	blake2s_final(&state, hash, curve->prec/8);
	
	__u32 r1[8];
	memcpy(&r1[0], hash, curve->prec/8);
	for(i=0; i < curve->prec/32; i++)
		r1[i] = htonl(r1[i]);
		
	if(0 == memcmp(r, &r1[0], curve->prec/8))
		printf("Signcryption worked\n");
	else
		printf("Verification failed\n");
		
	free_Point(&R);
	free(hash);
}
