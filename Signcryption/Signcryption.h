/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Signcryption.h
 * Author: user
 *
 * Created on 8. November 2019, 08:08
 */

#include "uECC.h"
#include <blake2.h>
#include "Node.h"
#include <cstring>

#ifndef SIGNCRYPTION_H
#define SIGNCRYPTION_H

class Signcryption {
public:
    Signcryption(Node *node);
    Signcryption(const Signcryption& orig);
    virtual ~Signcryption();
    void signcrypt(uint8_t *publicKey, uint8_t *msg, uint8_t msg_len);
    void Unsigncryption(uint8_t *publicKey, uint8_t *r, uint8_t*s, uint8_t *msg, uint8_t msg_len);
    uint8_t *msg;
    uint8_t msg_len;
    uint8_t *r;
    uint8_t *s;
private:
    uint8_t *h_key;
    uint8_t *e_key;
    Node *node;
    

};

#endif /* SIGNCRYPTION_H */

