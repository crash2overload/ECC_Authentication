/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ECDSA.h
 * Author: user
 *
 * Created on 18. November 2019, 08:07
 */

#ifndef ECDSA_H
#define ECDSA_H

#include <blake2.h>
#include "Node.h"
#include "uECC_vli.h"


class ECDSA {
public:
    ECDSA(Node *node);
    ECDSA(const ECDSA& orig);
    virtual ~ECDSA();
    void Sign(uint8_t *msg, uint8_t msg_len, uint8_t *r, uint8_t *s);
    void Verify(uint8_t *msg, uint8_t msg_len, uint8_t *r, uint8_t *s, uint8_t *publicKey);
    
    uint8_t *r;
    uint8_t *s;
private:
    Node *node;
    
    uint8_t *k;
    blake2s_state state;

};

#endif /* ECDSA_H */

