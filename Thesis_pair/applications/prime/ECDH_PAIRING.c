#include "../../sources/pbc/include/pbc.h"
#include "../../sources/pbc/include/pbc_test.h"

typedef struct Node{
	element_t privateKey;
	element_t publicKey;
	element_t sharedKey;
	element_t pairing;
}Node_t;

void init_Node(Node_t *node, pairing_t pairing) {
	element_init_G1(node->publicKey, pairing);
	element_init_Zr(node->privateKey, pairing);
	element_init_GT(node->sharedKey, pairing);
	element_init_GT(node->pairing, pairing);
}

void generate_Keys(Node_t *node, pairing_t pairing, element_t generator) {
	element_random(node->privateKey);
	element_mul_zn(node->publicKey, generator, node->privateKey);	
}

void clear_Node(Node_t *node) {
	element_clear(node->privateKey);
	element_clear(node->sharedKey);
	element_clear(node->publicKey);
	element_clear(node->pairing);
}

int main(int argc, char **argv) {
  
	pairing_t pairing;
	double time1, time2;
  	Node_t Alice, Bob;
	element_t G;
  
	pbc_demo_pairing_init(pairing, argc, argv);
	if (!pairing_is_symmetric(pairing)) pbc_die("pairing must be symmetric");

	element_init_G1(G, pairing);
	init_Node(&Alice, pairing);
	init_Node(&Bob, pairing);
  
  
	time1 = pbc_get_time();
	printf("ECDH via Pairing.\n");
	element_random(G);
	generate_Keys(&Bob, pairing, G);
	generate_Keys(&Alice, pairing, G);
	
	printf("Alice sends Bob  U_A =\n");
	element_printf("%B\n", Alice.publicKey);
  
	printf("Bob sends Alice  U_B =\n");
	element_printf("%B\n", Bob.publicKey);

	/* e(U_B, G) */
	element_pairing(Alice.pairing, Bob.publicKey, G);
	/* e(U_B, G) ^ d_A */
	element_pow_zn(Alice.sharedKey, Alice.pairing, Alice.privateKey);
	element_printf("k_A = %B\n", Alice.sharedKey);
	
	/* e(U_A, G) */
	element_pairing(Bob.pairing, Alice.publicKey, G);
	/* e(U_A, G) ^ d_B */
	element_pow_zn(Bob.sharedKey, Bob.pairing, Bob.privateKey);
	element_printf("k_B = %B\n", Bob.sharedKey);

	if(! element_cmp(Alice.sharedKey, Bob.sharedKey))
		printf("Shared key equal\n");
	else
		printf("Shared key different\n");
	
	time2 = pbc_get_time();
	printf("All time = %fs\n", time2 - time1);

	
	clear_Node(&Alice);
	clear_Node(&Bob);
	element_clear(G);
	pairing_clear(pairing);

  return 0;
}
