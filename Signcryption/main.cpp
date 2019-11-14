/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: user
 *
 * Created on 7. November 2019, 12:03
 */


#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include "Signcryption.h"
#include "Node.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    printf("Hallo\n\n");
    
    Node Alice(192), Bob(192);
    Signcryption s_Alice(&Alice);
    Signcryption s_Bob(&Bob);
    
    uint8_t msg[2] = {0x0};
    s_Alice.signcrypt(Bob.get_publicKey(), &msg[0], 2);
    s_Bob.Unsigncryption(Alice.get_publicKey(), s_Alice.r, s_Alice.s, &msg[0], 2);
    
    if(memcmp(s_Bob.r, s_Alice.r, 16) != 0){
        printf("not");
    } else {
        printf("yes");
    }
    //printf("%02x", Alice.get_SharedKey()[0]);
    
    return 0;
}

