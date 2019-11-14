/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Signcryption.cpp
 * Author: user
 * 
 * Created on 8. November 2019, 08:08
 */

#include <stdlib.h>
#include <stdio.h>
#include <cstring>

#include "Signcryption.h"
#include "uECC.h"
#include "uECC_vli.h"
#include "aes.hpp"


Signcryption::Signcryption(Node *node) {
    this->node = node;
    r = (uint8_t*) malloc(uECC_curve_private_key_size(node->get_curve()));
    s = (uint8_t*) malloc(uECC_curve_private_key_size(node->get_curve()));
    h_key = (uint8_t*) malloc(uECC_curve_private_key_size(node->get_curve())/2);
    e_key = (uint8_t*) malloc(uECC_curve_private_key_size(node->get_curve())/2);
}

Signcryption::Signcryption(const Signcryption& orig) {
}

Signcryption::~Signcryption() {
    //free(this->node);
    free(r);
    free(s);
    free(h_key);
    free(e_key);
}

void Signcryption::signcrypt(uint8_t *publicKey, uint8_t *msg, uint8_t msg_len){
    
    uint8_t *tempKey = (uint8_t*) malloc(uECC_curve_private_key_size(node->get_curve()));
    uECC_make_private_key(&tempKey[0], node->get_curve());
    
    uECC_shared_secret2(publicKey, tempKey, node->get_SharedKey(), node->get_curve());
        
    blake2b_state state;
    blake2b_init(&state, 32);
    blake2b_update(&state, node->get_SharedKey(), uECC_curve_public_key_size(node->get_curve()));
    blake2b_final(&state, node->get_SharedKey(), 32);
    
    
    std::memcpy(h_key, node->get_SharedKey(), 16);
    std::memcpy(e_key, node->get_SharedKey()+16, 16);
    
    blake2b(r, msg, h_key, 16, msg_len, 24);
    
    
    printf("\n\n");
    printf("\nh_key: \n");
    for(int i = 0; i < uECC_curve_private_key_size(node->get_curve())/2; i++)
		printf("%02x", h_key[i]);
    printf("\n\n");
    printf("\ne_key: \n");
    for(int i = 0; i < 16; i++)
		printf("%02x", e_key[i]);
    printf("\n\n");
    printf("Nachricht: \n");
    for(int i = 0; i < msg_len; i++)
            printf("%02x ", msg[i]);
    
    uint8_t iv=0x0;
    struct AES_ctx ctx;
    
    AES_init_ctx_iv(&ctx, e_key, &iv);
    AES_CTR_xcrypt_buffer(&ctx, &msg[0], msg_len);
    
    modularAdd2(node->get_privateKey(), r, s, node->get_curve());
    modularInv2(s, s, node->get_curve());
    modularMult2(tempKey, s, s, node->get_curve());
    
}

void Signcryption::Unsigncryption(uint8_t *publicKey, uint8_t *r, uint8_t *s, uint8_t *msg, uint8_t msg_len){
    
    memcpy(this->s, s, uECC_curve_private_key_size(node->get_curve()));
        
    modularMult2(this->s, node->get_privateKey(), this->s, node->get_curve());
    uECC_compute_public_key(r, node->get_SharedKey(), node->get_curve());
    
    EllipticAdd(node->get_SharedKey(), publicKey, node->get_SharedKey(), node->get_curve());
    uECC_shared_secret2(node->get_SharedKey(), this->s, node->get_SharedKey(), node->get_curve());
    
    blake2b_state state;
    blake2b_init(&state, 32);
    blake2b_update(&state, node->get_SharedKey(), uECC_curve_public_key_size(node->get_curve()));
    blake2b_final(&state, node->get_SharedKey(), 32);
    
    std::memcpy(h_key, node->get_SharedKey(), 16);
    std::memcpy(e_key, node->get_SharedKey()+16, 16);

    printf("\nh_key: \n");
    for(int i = 0; i < 16; i++)
		printf("%02x", h_key[i]);
    printf("\n\n");
    printf("\ne_key: \n");
    for(int i = 0; i < 16; i++)
		printf("%02x", e_key[i]);
    printf("\n\n");
    printf("Nachricht: \n");
    
    printf("Nachricht: \n");
    for(int i = 0; i < msg_len; i++)
            printf("%02x ", msg[i]);
    
    uint8_t iv=0x0;
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, e_key, &iv);
    AES_CTR_xcrypt_buffer(&ctx, msg, msg_len);
    
    printf("Nachricht: \n");
    for(int i = 0; i < msg_len; i++)
            printf("%02x ", msg[i]);
    
    blake2b_init_key(&state, uECC_curve_private_key_size(node->get_curve()), &h_key[0], uECC_curve_private_key_size(node->get_curve())/2);
    blake2b_update(&state, msg, msg_len);
    blake2b_final(&state, &this->r[0], uECC_curve_private_key_size(node->get_curve()));
    
    
}

