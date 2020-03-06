#include "../sources/pbc/include/pbc.h"
#include "../sources/pbc/include/pbc_test.h"

typedef struct Node{
	element_t privateKey;
	element_t publicKey;
	element_t sharedKey;
	element_t pairing;
}Node_t;

typedef struct Message{
	element_t sig;
	element_t hash;
}Message_t;

void init_symmetric_Node(Node_t *node, pairing_t pairing) {
	element_init_G1(node->publicKey, pairing);
	element_init_Zr(node->privateKey, pairing);
	element_init_GT(node->sharedKey, pairing);
	element_init_GT(node->pairing, pairing);
}

void init_asymmetric_Node(Node_t *node, pairing_t pairing) {
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

void init_Message(Message_t *msg, pairing_t pairing) {
	element_init_G1(msg->sig, pairing);
	element_init_G1(msg->hash, pairing);
}

void clear_Message(Message_t *msg) {
	element_clear(msg->sig);
	element_clear(msg->hash);
}

