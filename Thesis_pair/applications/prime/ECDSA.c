#include "../../include/pairing_util.h"

int main(int argc, char **argv) {
	
	pairing_t pairing;
  
	Node_t Alice;
	Message_t msg;
	double time1, time2;
	element_t g, hash, sig, k, xr, s, u1, u2;
	element_t temp1, temp2, R;

	pbc_demo_pairing_init(pairing, argc, argv);

	init_symmetric_Node(&Alice, pairing);
	//init_Message(&msg, pairing);
	
	element_init_G1(g, pairing);
	element_init_G1(sig, pairing);
	element_init_Zr(hash, pairing);
	element_init_Zr(k, pairing);
	element_init_Zr(s, pairing);
	element_init_Zr(u2, pairing);
	element_init_Zr(u1, pairing);
	element_init_Zr(xr, pairing);
	element_init_G1(temp1, pairing);
	element_init_G1(temp2, pairing);
	element_init_G1(R, pairing);

	printf("ECDSA \n");
	time1 = pbc_get_time();
	/*  Get Generator */ 
	element_random(g);
	element_printf("Generator %B\n", g);
		
	//generate keys
	generate_Keys(&Alice, pairing, g);
	  	
	element_random(k);
	element_mul_zn(R, g, k);
	
	int n = pairing_length_in_bytes_x_only_G1(pairing);
    element_from_hash(hash, "MeineGeheimeNachricht", n);
	element_printf("Message hashed = %B\n", hash);
    unsigned char *data = pbc_malloc(n);
    element_to_bytes_x_only(data, R);
    element_from_bytes(xr, data);
    pbc_free(data);
    
    element_mul(s, xr, Alice.privateKey); 
    element_add(s, s, hash);
    element_invert(k, k);
    element_mul(s, s, k);
    
    element_printf("s = %B\n\n", s);
    element_printf("R = %B\n\n", R);
    element_printf("xr = %B\n\n", xr);
	
	/* Verification  */
	element_invert(s, s);
	element_mul(u1, hash, s);
	element_mul(u2, xr, s);
	
	element_mul_zn(temp1, g, u1);
	element_mul_zn(temp2, Alice.publicKey, u2);
	element_add(temp1, temp1, temp2);
	element_printf("erer = %B\n", temp1);
	
	if (!element_cmp(temp1, R)) {
		printf("Signature verified\n");
	} else {
		printf("Verification failed\n");
	}
	time2 = pbc_get_time();
	printf("All time = %fs\n", time2 - time1);
	
	/* Free Memory */
	element_clear(g);
	element_clear(temp1);
	element_clear(temp2);
	element_clear(k);
	element_clear(xr);
	element_clear(sig);
	element_clear(hash);
	element_clear(R);
	clear_Node(&Alice);
		
	pairing_clear(pairing);
	  
	return 0;
}
