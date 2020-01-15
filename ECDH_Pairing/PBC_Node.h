/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PBC_Node.h
 * Author: user
 *
 * Created on 15. Januar 2020, 14:55
 */

#ifndef PBC_NODE_H
#define PBC_NODE_H

#include <pbc/pbc.h>

class PBC_Node {
public:
    PBC_Node();
    PBC_Node(const PBC_Node& orig);
    virtual ~PBC_Node();
private:
    pairing_t pairing;
    element_t P, d, U;

};

#endif /* PBC_NODE_H */

