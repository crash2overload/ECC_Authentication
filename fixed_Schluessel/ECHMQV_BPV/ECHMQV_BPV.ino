#include <uECC_vli.h>
#include <uECC.h>
#include <BLAKE2s.h>
//#include "keys_192.h"
#include "Arazi_BPV.h"


static int RNG2(uint8_t *dest, unsigned size) {

  while (size) {
    uint8_t val = 0;
    val = rand() % 255;
    *dest = val;
    ++dest;
    --size;
  }
  return 1;
}

void ECHMQV_get_shared_Secret(struct Node * node, uint8_t * emphermalPublicKey, const struct uECC_Curve_t * curve){
  
  uint8_t authID2[24];
  
  blake.reset();
  blake.update(node->tempPub, uECC_curve_public_key_size(curve));
  blake.finalize(node->authID, uECC_curve_private_key_size(curve));

  blake.reset(sizeof(authID2));
  blake.update(emphermalPublicKey, uECC_curve_public_key_size(curve));
  blake.finalize(authID2, sizeof(authID2));

  uECC_mult_add(node->sharedSecret, authID2, emphermalPublicKey, node->sharedSecret, curve);
  
  modularMultAdd(node->privateKey, node->authID, node->tempPriv, node->tempPriv, curve);

  uECC_shared_secret2(node->sharedSecret, node->tempPriv, node->sharedSecret, curve);
}


void setup() {
  Serial.begin(115200);
  Serial.print("ECHMQV + BPV\n");
  uECC_set_rng(&RNG2);

}

void loop() {
  
  const struct uECC_Curve_t * curve = uECC_secp192r1();
  Node Alice, Bob;
  uint8_t privateCA[24];
  uint8_t publicCA[48];

  unsigned long a, b, timebob, timealice;

  uECC_make_key(publicCA, privateCA, curve);
  uECC_make_key(Alice.publicKey, Alice.privateKey, curve);
  uECC_make_key(Bob.publicKey, Bob.privateKey, curve);

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
  getTempKeys(Alice.tempPub, Alice.tempPriv, curve); 
  b = micros();
  timealice = timealice + (b - a);

    
  a = micros();
  getTempKeys(Bob.tempPub, Bob.tempPriv, curve);
  b = micros();
  timebob = timebob + (b - a);


  a = micros();
  getTempKeys(Bob.tempPub, Bob.tempPriv, curve);
  b = micros();
  timebob = timebob + (b - a);

  uECC_mult_add(Bob.publicKey, Bob.ID, publicCA, Alice.sharedSecret, curve);

  uECC_mult_add(Alice.publicKey, Alice.ID, publicCA, Bob.sharedSecret, curve);
  
  a = micros();
  blake.reset(sizeof(Alice.authID));
  blake.update(Alice.tempPub, sizeof(Alice.tempPub));
  blake.finalize(Alice.authID, sizeof(Alice.authID));

  blake.reset(sizeof(Bob.authID));
  blake.update(Bob.tempPub, sizeof(Bob.tempPub));
  blake.finalize(Bob.authID, sizeof(Bob.authID));
    
  b = micros();
  timebob = timebob + microsecondsToClockCycles(b - a);

   a = micros();
  ECHMQV_get_shared_Secret(&Alice, Bob.tempPub, curve);
  b = micros();
  timealice = timealice + microsecondsToClockCycles(b - a);

  a = micros();
  uECC_shared_secret2(Bob.sharedSecret, Alice.authID, Bob.sharedSecret, curve);
  EllipticAdd(Bob.sharedSecret, Alice.tempPub, Bob.sharedSecret, curve);

  modularMultAdd(Bob.privateKey, Bob.authID, Bob.tempPriv, Bob.tempPriv, curve);
  uECC_shared_secret2(Bob.sharedSecret, Bob.tempPriv, Bob.sharedSecret, curve);
  
  b = micros();
  timebob = timebob + microsecondsToClockCycles(b - a);

  Serial.print("ECHMQV+BPV Alice in: "); Serial.println(timealice);
  Serial.print("ECHMQV+BPV Bob in: "); Serial.println(timebob);

  if (memcmp(Alice.sharedSecret, Bob.sharedSecret, 24) != 0) {
    Serial.println("Gemeinsamer Schlüssel verschieden");
  } else {    
    Serial.println("Gemeinsamer Schlüssel identisch");
  }

}
