#include <uECC_vli.h>
#include <uECC.h>
#include <BLAKE2s.h>
#include "Arazi.h"

static int RNG2(uint8_t *dest, unsigned size) {

  while (size) {
    uint8_t val = 0;
    val = rand() % 256;
    *dest = val;
    ++dest;
    --size;
  }
  return 1;
}

BLAKE2s blake;

void AQ_make_privateKey(struct Node * node, uint8_t * privateCA, const struct uECC_Curve_t * curve){
   /* hash = h(ID,U) */
  blake.reset();
  blake.update(node->publicKey, sizeof(node->publicKey));
  blake.finalize(node->ID, sizeof(node->ID));  
  /* (h(ID,U)*b + d) mod ord(G) */
  modularMultAdd(node->ID, node->privateKey, privateCA, node->privateKey, curve);
  
}

void AQ_shared_Secret(uint8_t * ID, uint8_t * publicKey, uint8_t * emphermalPublicKey, struct Node * node, uint8_t * publicCA, const struct uECC_Curve_t * curve) {

  uECC_mult_add(publicKey, ID, publicCA, node->sharedSecret, curve);
  uECC_shared_secret2(node->sharedSecret, node->privateKey, node->sharedSecret, curve);
  uECC_mult_add(emphermalPublicKey, node->tempPriv, node->sharedSecret, node->sharedSecret, curve);
}

void setup() {
  Serial.begin(115200);
  Serial.print("Arazi\n");
  uECC_set_rng(&RNG2);

}

void loop() {

  const struct uECC_Curve_t * curve = uECC_secp224r1();
  Node Alice, Bob;
  Alice.publicKey = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  Alice.privateKey = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Alice.tempPub = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  Alice.tempPriv = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Alice.ID = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Alice.sharedSecret = (uint8_t*)malloc(uECC_curve_public_key_size(curve));

  Bob.publicKey = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  Bob.privateKey = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Bob.tempPub = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  Bob.tempPriv = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Bob.ID = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Bob.sharedSecret = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  
  uint8_t *privateCA = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  uint8_t *publicCA = (uint8_t*)malloc(uECC_curve_public_key_size(curve));

  unsigned long a, b, timebob, timealice;

  uECC_make_key(publicCA, privateCA, curve);
  uECC_make_key(Bob.publicKey, Bob.privateKey, curve);
  uECC_make_key(Alice.publicKey, Alice.privateKey, curve);

 
  a = micros();
  AQ_make_privateKey(&Bob, privateCA, curve);
  b = micros();
  timebob = microsecondsToClockCycles(b - a);

  /* (h(ID_A,U_A)*b_A + d) mod ord(G) */
  a = micros();
  AQ_make_privateKey(&Alice, privateCA, curve); 
  b = micros();
  timealice = microsecondsToClockCycles(b - a);
  
   a = micros();
  /* E_A = p_A*G */
  uECC_make_key(Alice.tempPub, Alice.tempPriv, curve);
  b = micros();
  timealice = timealice + microsecondsToClockCycles(b - a);

  a = micros();
  /* E_B = p_B*G */
  uECC_make_key(Bob.tempPub, Bob.tempPriv, curve);
  b = micros();
  timebob = timebob + microsecondsToClockCycles(b - a);

  /* d_A X (h(ID_B||U_B) x U_B + D) + p_A x E_B */
   /* h(ID_B||U_B) x U_B + D calculated Offline*/
  uECC_mult_add(Bob.publicKey, Bob.ID, publicCA, Alice.sharedSecret, curve);
  /* d_A X (h(ID_B||U_B) x U_B + D) */
  uECC_shared_secret2(Alice.sharedSecret, Alice.privateKey, Alice.sharedSecret, curve);
  /* d_A X (h(ID_B||U_B) x U_B + D) + p_A x E_B */
  a = micros();
  uECC_mult_add(Bob.tempPub, Alice.tempPriv, Alice.sharedSecret, Alice.sharedSecret, curve);
  b = micros();
  timealice = timealice + microsecondsToClockCycles(b - a);


  /* d_B X (h(ID_A||U_A) x U_A + D) + p_B x E_A */
  /* d_B X (h(ID_A||U_A) x U_A + D) calculated Online */
  a = micros();
  AQ_shared_Secret(Alice.ID, Alice.publicKey, Alice.tempPub, &Bob, publicCA, curve);
  b = micros();
  timebob = timebob + microsecondsToClockCycles(b - a);

  Serial.print("Arazi (Alice) in: "); Serial.println(timealice);
  Serial.print("Arazi (Bob) in: "); Serial.println(timebob);


  if (memcmp(Alice.sharedSecret, Bob.sharedSecret, 24) != 0) {
    Serial.print("Shared secrets are not identical!\n");
  } else {
    Serial.print("Shared secrets are identical\n");
  }

  free(privateCA);
  free(publicCA);
  free(Alice.publicKey);
  free(Alice.privateKey);
  free(Alice.tempPub);
  free(Alice.tempPriv);
  free(Alice.ID);
  free(Alice.sharedSecret);
  free(Bob.privateKey);
  free(Bob.publicKey);
  free(Bob.tempPriv);
  free(Bob.tempPub);
  free(Bob.ID);
  free(Bob.sharedSecret);
}
