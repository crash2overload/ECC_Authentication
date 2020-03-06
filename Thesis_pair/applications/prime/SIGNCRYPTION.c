#include "../../include/pairing_util.h"
#include <blake2.h>
#include <string.h>
#include "../../include/aes.h"

int main(int argc, char **argv) {
	
	pairing_t pairing;
  
	Node_t Alice, Bob;
	double time1, time2;
	element_t g, sig, p, r, r2, s;
	element_t S, R;

	pbc_demo_pairing_init(pairing, argc, argv);

	init_symmetric_Node(&Alice, pairing);
	init_symmetric_Node(&Bob, pairing);
	
	element_init_G1(g, pairing);
	element_init_G1(sig, pairing);
	element_init_Zr(p, pairing);
	element_init_Zr(r, pairing);
	element_init_Zr(r2, pairing);
	element_init_Zr(s, pairing);
	element_init_G1(S, pairing);
	element_init_G1(R, pairing);

	printf("Signcryption \n");
	time1 = pbc_get_time();
	/*  Get Generator */ 
	element_random(g);
	element_random(p);
	
	//generate keys
	generate_Keys(&Alice, pairing, g);
	generate_Keys(&Bob, pairing, g);
	
	/******************** Signcryption  **********************/
	
	/*  S = p_A * U_B  */
	element_mul_zn(S, Bob.publicKey, p);
		
	/* Symmetric and MAC key */
	int n = pairing_length_in_bytes_x_only_G1(pairing);
	unsigned char *data = pbc_malloc(n);
    element_to_bytes_x_only(data, S);
    
    unsigned char *k_m = pbc_malloc(n/2);
    unsigned char *k_e = pbc_malloc(n/2);
    memcpy(k_m, data, n/2);
    memcpy(k_e, data + (n/2), n/2);
    
    unsigned char msg[14] = "MeineNachricht";
	unsigned char hash[32];   	
	blake2s_state state;
	blake2s_init_key(&state, 32, &k_m[0], n/2);
	blake2s_update(&state, msg, 14);
	blake2s_final(&state, &hash[0], 32);
	
	printf("Message plain ");
	for (int i = 0; i < 14; i++) {
		printf("%02X", msg[i]);
    }
    printf("\n");
    	
	//generate element in G1 (elliptic curve Point) from a hash
	element_from_hash(r, hash, 32);
	element_printf("r = %B\n", r);
	
	element_add(s, r, Alice.privateKey);
	element_invert(s, s);
	element_mul_zn(s, p, s);
	element_printf("s = %B\n", s);
	
	uint8_t iv=0x0;
	struct AES_ctx ctx;
	AES_init_ctx(&ctx, &k_e[0]);
	AES_init_ctx_iv(&ctx, &k_e[0], &iv);
	AES_CTR_xcrypt_buffer(&ctx, msg, 14);
	
	printf("c = ");
	for (int i = 0; i < 14; i++) {
		printf("%02X", msg[i]);
    }
    printf("\n");
    
    /**************  Unsigncryption ***********************/
	
	element_mul_zn(s, s, Bob.privateKey);
	element_mul_zn(R, g, r);
	element_add(R, R, Alice.publicKey);
	element_mul_zn(R, R, s);
	
    element_to_bytes_x_only(data, S);
    
    unsigned char *k_m_b = pbc_malloc(n/2);
    unsigned char *k_e_b = pbc_malloc(n/2);
    memcpy(k_m_b, data, n/2);
    memcpy(k_e_b, data + (n/2), n/2);
    
    iv = 0;
	AES_init_ctx(&ctx, &k_e_b[0]);
	AES_init_ctx_iv(&ctx, &k_e_b[0], &iv);
	AES_CTR_xcrypt_buffer(&ctx, msg, 14);
	
	printf("Message decrypted  c = ");
	for (int i = 0; i < 14; i++) {
		printf("%02X", msg[i]);
    }
    printf("\n");
    
    blake2s_init_key(&state, 32, &k_m_b[0], n/2);
	blake2s_update(&state, msg, 14);
	blake2s_final(&state, &hash[0], 32);
	
	element_from_hash(r2, hash, 32);
	
	if (!element_cmp(r, r2)) {
		printf("Signature verified\n");
	} else {
		printf("Verification failed\n");
	}
	
	printf("\n");
	time2 = pbc_get_time();
	printf("All time = %fs\n", time2 - time1);
	
	/* Free Memory */
	pbc_free(data);
	pbc_free(k_e);
	pbc_free(k_m);
	pbc_free(k_e_b);
	pbc_free(k_m_b);
	
	element_clear(g);
	element_clear(p);
	element_clear(r2);
	element_clear(r);
	element_clear(S);
	element_clear(R);

	clear_Node(&Alice);
	clear_Node(&Bob);
		
	pairing_clear(pairing);
	  
	return 0;
}
