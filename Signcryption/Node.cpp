/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Node.cpp
 * Author: user
 * 
 * Created on 8. November 2019, 08:19
 */

#include "Node.h"
#include "stdlib.h"
#include "iostream"

Node::Node() {
}

Node::Node(uint8_t prec){
    curve = uECC_secp192r1();
    this->privateKey = (uint8_t*) malloc(uECC_curve_private_key_size(curve));
    this->publicKey = (uint8_t*) malloc(uECC_curve_public_key_size(curve));
    this->sharedSecret = (uint8_t*) malloc(uECC_curve_public_key_size(curve));
    if (!uECC_make_key(this->publicKey, this->privateKey, curve))
        std::cout << "Error occurred!";
}

Node::Node(const Node& orig) {
}

Node::~Node() {
    free(privateKey);
    free(publicKey);
    free(sharedSecret);
}

void Node::make_SharedSecret(uint8_t *publicKey){
    uECC_shared_secret(privateKey, publicKey, sharedSecret, curve);
}

uint8_t* Node::get_SharedKey(){
    return sharedSecret;
}

uint8_t* Node::get_publicKey(){
    return publicKey;
}

uint8_t* Node::get_privateKey(){
    return privateKey;
}

const uECC_Curve_t* Node::get_curve(){
    return curve;
}

extern "C" {

static int RNG(uint8_t *dest, unsigned size) {
  // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of 
  // random noise). This can take a long time to generate random data if the result of analogRead(0) 
  // doesn't change very frequently.
  while (size) {
    uint8_t val = 0;
    for (unsigned i = 0; i < 8; ++i) {
      //int init = analogRead(0);
      int count = 0;
      //while (analogRead(0) == init) {
      //  ++count;
      //}
      
      if (count == 0) {
         //val = (val << 1) | (init & 0x01);
      } else {
         val = (val << 1) | (count & 0x01);
      }
    }
    *dest = val;
    ++dest;
    --size;
  }
  // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
  return 1;
}

}  // extern "C"

