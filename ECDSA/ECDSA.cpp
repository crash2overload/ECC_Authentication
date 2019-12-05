/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ECDSA.cpp
 * Author: user
 * 
 * Created on 18. November 2019, 08:07
 */

#include "ECDSA.h"
#include <cstring>
#include <stdlib.h>
#include <stdio.h>

ECDSA::ECDSA(Node *node) {
    this->node = node;
    r = (uint8_t*) malloc(uECC_curve_private_key_size(node->get_curve()));
    s = (uint8_t*) malloc(uECC_curve_private_key_size(node->get_curve()));
    k = (uint8_t*) malloc(uECC_curve_private_key_size(node->get_curve()));
    
}

ECDSA::ECDSA(const ECDSA& orig) {
}

ECDSA::~ECDSA() {
    free(r);
    free(s);
    free(k);
}

void ECDSA::Sign(uint8_t *msg, uint8_t msg_len, uint8_t *r, uint8_t *s){
    
    uint8_t *e;
    Node tempNode(192);
   
    //Get hashed message
    e = (uint8_t*) malloc(uECC_curve_private_key_size(node->get_curve()));
    blake2s_init(&state, uECC_curve_private_key_size(node->get_curve()));
    blake2s_update(&state, msg, msg_len);
    blake2s_final(&state, e, uECC_curve_private_key_size(node->get_curve()));
    
    //Get temporary key
    
    modularInv2(tempNode.get_privateKey(), tempNode.get_privateKey(), tempNode.get_curve());
    modularMult2(tempNode.get_publicKey(), node->get_privateKey(), tempNode.get_SharedKey(), tempNode.get_curve());
    modularAdd2(tempNode.get_SharedKey(), e, tempNode.get_SharedKey(), tempNode.get_curve());
    modularMult2(tempNode.get_privateKey(), tempNode.get_SharedKey(), tempNode.get_SharedKey(), tempNode.get_curve());
    
    memcpy(r, tempNode.get_publicKey(), uECC_curve_private_key_size(node->get_curve()));
    memcpy(s, tempNode.get_SharedKey(), uECC_curve_private_key_size(node->get_curve()));
    
    printf("\n r: \n");
    for(int i = 0; i < 16; i++)
		printf("%02x", s[i]);
}

void ECDSA::Verify(uint8_t *msg, uint8_t msg_len, uint8_t *r, uint8_t *s, uint8_t *publicKey){
    
    //Get hashed message
    uint8_t *e;
    Node tempNode(192);
    e = (uint8_t*) malloc(uECC_curve_private_key_size(node->get_curve()));
    blake2s_init(&state, uECC_curve_private_key_size(node->get_curve()));
    blake2s_update(&state, msg, msg_len);
    blake2s_final(&state, e, uECC_curve_private_key_size(node->get_curve()));
    
    printf("\n e: \n");
    for(int i = 0; i < 24; i++)
		printf("%02x", e[i]);
    
    modularInv2(s, this->s, node->get_curve());
    
    printf("\n 1/s: \n");
    for(int i = 0; i < 24; i++)
		printf("%02x", this->s[i]);
    modularMult2(e, this->s, this->k, node->get_curve());
    printf("\n se: \n");
    for(int i = 0; i < 24; i++)
		printf("%02x", this->k[i]);
    
    modularMult2(r, this->s, this->r, node->get_curve());
    
    uECC_compute_public_key(this->k, tempNode.get_publicKey(), node->get_curve());
    uECC_shared_secret(publicKey, this->r, tempNode.get_SharedKey(), node->get_curve()); 
    EllipticAdd(tempNode.get_publicKey(), tempNode.get_SharedKey(), tempNode.get_SharedKey(), node->get_curve());
    
     printf("\n SHARED: \n");
    for(int i = 0; i < 16; i++)
		printf("%02x", tempNode.get_SharedKey()[i]);
    printf("\n\n");
    printf("\n r: \n");
    for(int i = 0; i < 16; i++)
		printf("%02x", r[i]);
    printf("\n\n");
    
}

