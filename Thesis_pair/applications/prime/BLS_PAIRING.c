#include "../../include/pairing_util.h"



int main(int argc, char **argv) {
	
	pairing_t pairing;
  
	Node_t Alice;
	Message_t msg;
	double time1, time2;
	element_t g;
	element_t temp1, temp2;

	pbc_demo_pairing_init(pairing, argc, argv);

	init_asymmetric_Node(&Alice, pairing);
	init_Message(&msg, pairing);
	
	element_init_G2(g, pairing);
	element_init_GT(temp1, pairing);
	element_init_GT(temp2, pairing);

	printf("BLS Signatur\n");
	time1 = pbc_get_time();
	/*  Get Generator */ 
	element_random(g);
		
	//generate keys
	generate_Keys(&Alice, pairing, g);
	  	
	//generate element in G1 (elliptic curve Point) from a hash
	element_from_hash(msg.hash, "MeineGeheimeNachricht", 21);
	element_printf("Message hashed = %B\n", msg.hash);
	
	/* Get Signatur d_A*h(m) */
	element_pow_zn(msg.sig, msg.hash, Alice.privateKey);
	element_printf("signature = %B\n", msg.sig);
	
	//verification part 1
	element_pairing(temp1, msg.sig, g);
	element_printf("e(sig, g) = %B\n", temp1);
	
	//verification part 2
	//should match above
	element_pairing(temp2, msg.hash, Alice.publicKey);
	element_printf("e(h(m), U_A) = %B\n", temp2);
	
	if (!element_cmp(temp1, temp2)) {
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
	
	clear_Node(&Alice);
	clear_Message(&msg);
	
	pairing_clear(pairing);
	  
	return 0;
}
