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
    printf("Hallo");
    
    Node Alice(192), Bob(192);
    Signcryption s_Alice(&Alice);
    std::string message = "Hallo";
    uint8_t msg[2] = {0x01,0x10};
    s_Alice.signcrypt(Bob.get_publicKey(), &msg[0], 2);
    
    //printf("%02x", Alice.get_SharedKey()[0]);
    
    return 0;
}

