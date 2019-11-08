/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Node.h
 * Author: user
 *
 * Created on 8. November 2019, 08:19
 */
#include "uECC.h"

#ifndef NODE_H
#define NODE_H

class Node {
public:
    Node();
    Node(uint8_t prec);
    Node(const Node& orig);
    virtual ~Node();
    void make_SharedSecret(uint8_t *publicKey);
    uint8_t* get_SharedKey();
    uint8_t* get_publicKey();
    const uECC_Curve_t* get_curve();
    uint8_t* get_privateKey();
private:
    uint8_t *sharedSecret;
    uint8_t *publicKey;
    uint8_t *privateKey;
    const struct uECC_Curve_t * curve;

};

#endif /* NODE_H */

