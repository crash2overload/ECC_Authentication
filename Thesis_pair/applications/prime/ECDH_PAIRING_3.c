#include "../../include/pairing_util.h"

int main(int argc, char **argv) {
  
	pairing_t pairing;
	double time1, time2;
  	Node_t Alice, Bob, Carol;
	element_t G;
  
	pbc_demo_pairing_init(pairing, argc, argv);
	if (!pairing_is_symmetric(pairing)) pbc_die("pairing must be symmetric");

	element_init_G1(G, pairing);
	
	/* Initalize Nodes */
	init_symmetric_Node(&Alice, pairing);
	init_symmetric_Node(&Bob, pairing);
	init_symmetric_Node(&Carol, pairing);
  
  
	time1 = pbc_get_time();
	printf("3-Party-ECDH via Pairing.\n");
	/* Get a Generator */
	element_random(G);
	
	/* Create Keys */
	generate_Keys(&Bob, pairing, G);
	generate_Keys(&Alice, pairing, G);
	generate_Keys(&Carol, pairing, G);
	
	printf("Alice sends Bob and Carol  U_A =\n");
	element_printf("%B\n", Alice.publicKey);
  
	printf("Bob sends Alice and Carol U_B =\n");
	element_printf("%B\n", Bob.publicKey);
	
	printf("Carol sends Alice and Bob U_C =\n");
	element_printf("%B\n", Carol.publicKey);

	/* e(U_B, U_C) */
	element_pairing(Alice.pairing, Bob.publicKey, Carol.publicKey);
	/* e(U_B, U_C) ^ d_A */
	element_pow_zn(Alice.sharedKey, Alice.pairing, Alice.privateKey);
	element_printf("k_A = %B\n", Alice.sharedKey);
	
	/* e(U_A, U_C) */
	element_pairing(Bob.pairing, Alice.publicKey, Carol.publicKey);
	/* e(U_A, U_C) ^ d_B */
	element_pow_zn(Bob.sharedKey, Bob.pairing, Bob.privateKey);
	element_printf("k_B = %B\n", Bob.sharedKey);
	
	/* e(U_A, U_B) */
	element_pairing(Carol.pairing, Alice.publicKey, Bob.publicKey);
	/* e(U_A, U_B) ^ d_C */
	element_pow_zn(Carol.sharedKey, Carol.pairing, Carol.privateKey);
	element_printf("k_C = %B\n", Carol.sharedKey);

	if(! element_cmp(Alice.sharedKey, Bob.sharedKey))
	{		
		if(! element_cmp(Alice.sharedKey, Carol.sharedKey))
			printf("Shared key equal\n");
	}
	else
		printf("Shared key different\n");
	
	time2 = pbc_get_time();
	printf("All time = %fs\n", time2 - time1);

	
	clear_Node(&Alice);
	clear_Node(&Bob);
	clear_Node(&Carol);
	element_clear(G);
	pairing_clear(pairing);

  return 0;
}
