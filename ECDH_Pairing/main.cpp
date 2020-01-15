/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: user
 *
 * Created on 15. Januar 2020, 14:01
 */

#include <cstdlib>
#include <pbc/pbc.h>
#include <pbc/pbc_test.h>

using namespace std;

int main(int argc, char **argv) {
  pairing_t pairing;
  double time1, time2;
  element_t P, a, b, Ka, Kb, t1, t2, t4, t5;
  pbc_demo_pairing_init(pairing, argc, argv);
  if (!pairing_is_symmetric(pairing)) pbc_die("pairing must be symmetric");

  element_init_G1(P, pairing);
  element_init_G1(t1, pairing);
  element_init_G1(t2, pairing);

  element_init_Zr(a, pairing);
  element_init_Zr(b, pairing);

  element_init_GT(t4, pairing);
  element_init_GT(t5, pairing);
  element_init_GT(Ka, pairing);
  element_init_GT(Kb, pairing);

  //time1 = pbc_get_time();
  printf("ECDH via Pairing.\n");
  element_random(P);
  element_random(a);
  element_random(b);
  element_mul_zn(t1, P, a);
  printf("A sends B  aP\n");
  element_printf("aP = %B\n", t1);
  element_mul_zn(t2, P, b);
  printf("B sends A : bP\n");
  element_printf("bP = %B\n", t2);


  element_pairing(t4, t2, P);
  element_pow_zn(Ka, t4, a);
  element_printf("Ka = %B\n", Ka);
  element_pairing(t5, t1, P);
  element_pow_zn(Kb, t5, b);
  element_printf("Kb = %B\n", Kb);

  printf("Shared key K = Ka = Kb\n");
  //time2 = pbc_get_time();
  printf("All time = %fs\n", time2 - time1);


  element_clear(P);
  element_clear(a);
  element_clear(b);
  element_clear(Ka);
  element_clear(Kb);
  element_clear(t1);
  element_clear(t2);
  element_clear(t4);
  element_clear(t5);
  pairing_clear(pairing);

  return 0;
}

