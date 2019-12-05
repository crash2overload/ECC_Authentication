/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: user
 *
 * Created on 18. November 2019, 08:04
 */

#include <cstdlib>
#include "Node.h"
#include "ECDSA.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    
    Node Alice(192), Bob(192);
    ECDSA e_Alice(&Alice), e_Bob(&Bob);
    
    uint8_t msg[16] = {0};
    
    e_Alice.Sign(&msg[0], 16, e_Alice.r, e_Alice.s);
    
    e_Bob.Verify(&msg[0], 16, e_Alice.r, e_Alice.s, Alice.get_publicKey());

    return 0;
}

