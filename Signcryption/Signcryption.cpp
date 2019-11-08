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
    s =(uint8_t*) malloc(uECC_curve_private_key_size(node->get_curve()));
    h_key = (uint8_t*) malloc(uECC_curve_private_key_size(node->get_curve())/2);
    e_key = (uint8_t*) malloc(uECC_curve_private_key_size(node->get_curve())/2);
}

Signcryption::Signcryption(const Signcryption& orig) {
}

Signcryption::~Signcryption() {
}

void Signcryption::signcrypt(uint8_t *publicKey, uint8_t *msg, uint8_t msg_len){
    
    uint8_t *tempKey = (uint8_t*) malloc(uECC_curve_private_key_size(node->get_curve()));
    uECC_make_private_key(tempKey, node->get_curve());
    node->make_SharedSecret(publicKey);
    std::memcpy(h_key, &node->get_SharedKey()[0], uECC_curve_private_key_size(node->get_curve())/2);
    std::memcpy(e_key, &node->get_SharedKey()[uECC_curve_private_key_size(node->get_curve())/2], uECC_curve_private_key_size(node->get_curve())/2);
    
    blake2s_state state;
    blake2s_init_key(&state, uECC_curve_private_key_size(node->get_curve()), &h_key[0], 8);
    blake2s_update(&state, msg, msg_len);
    blake2s_final(&state, &r[0], uECC_curve_private_key_size(node->get_curve()));
    
    printf("HASH: \n");
    for(int i = 0; i < uECC_curve_private_key_size(node->get_curve()); i++)
		printf("%02x", r[i]);
    printf("\n\n");
    printf("Nachricht: \n");
    for(int i = 0; i < msg_len; i++)
            printf("%02x ", msg[i]);
    
    uint8_t iv=0x0;
    struct AES_ctx ctx;
    AES_init_ctx(&ctx, &e_key[0]);
    AES_init_ctx_iv(&ctx, &e_key[0], &iv);
    AES_CTR_xcrypt_buffer(&ctx, &msg[0], msg_len);
    
    printf("\n\n");
    printf("Nachricht verschlüsselt: \n");
    for(int i = 0; i < msg_len; i++)
            printf("%02x ", msg[i]);
    
    printf("\n\n");
    printf("Private Key: 0x");
    for(int i = 0; i < uECC_curve_private_key_size(node->get_curve()); i++)
            printf("%02x", node->get_privateKey()[i]);
    
    //printf("\n\n");
    printf("+ 0x");
    for(int i = 0; i < uECC_curve_private_key_size(node->get_curve()); i++)
            printf("%02x", r[i]);
    
    modularAdd2(node->get_privateKey(), r, s, node->get_curve());
    
    printf("\n\n");
    printf("Private Key: 0x");
    for(int i = 0; i < uECC_curve_private_key_size(node->get_curve()); i++)
            printf("%02x", s[i]);
    
    
}
void Signcryption::Unsigncryption(){
    
}

