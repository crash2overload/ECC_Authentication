#include <uECC_vli.h>
#include <uECC.h>
#include <BLAKE2s.h>
#include "ECHMQV.h"

BLAKE2s blake;

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

void ECHMQV_get_shared_Secret(struct Node * node, uint8_t * publicKey, uint8_t * emphermalPublicKey, const struct uECC_Curve_t * curve){
  
  uint8_t authID2[24];
  
  blake.reset();
  blake.update(node->tempPub, uECC_curve_public_key_size(curve));
  blake.finalize(node->authID, uECC_curve_private_key_size(curve));

  blake.reset(sizeof(authID2));
  blake.update(emphermalPublicKey, uECC_curve_public_key_size(curve));
  blake.finalize(authID2, sizeof(authID2));

  uECC_mult_add(publicKey, authID2, emphermalPublicKey, node->sharedSecret, curve);

  modularMultAdd(node->authID, node->privateKey, node->tempPriv, node->tempPriv, curve);
  uECC_shared_secret2(node->sharedSecret, node->tempPriv, node->sharedSecret, curve);
}

void setup() {
  Serial.begin(115200);
  Serial.print("ECHMQV\n");
  uECC_set_rng(&RNG2);

}

void loop() {
 
  const struct uECC_Curve_t * curve = uECC_secp192r1();
  Node Alice, Bob;
  uint8_t privateCA[24];
  uint8_t publicCA[48];

  unsigned long a, b, timebob, timealice;

  uECC_make_key(publicCA, privateCA, curve);
  uECC_make_key(Bob.publicKey, Bob.privateKey, curve);
  uECC_make_key(Alice.publicKey, Alice.privateKey, curve);

  blake.reset(sizeof(Alice.ID));
  blake.update(Alice.publicKey, sizeof(Alice.publicKey));
  blake.finalize(Alice.ID, sizeof(Alice.ID));

  blake.reset(sizeof(Bob.ID));
  blake.update(Bob.publicKey, sizeof(Bob.publicKey));
  blake.finalize(Bob.ID, sizeof(Bob.ID));

  a = micros();
  uECC_make_key(Alice.tempPub, Alice.tempPriv, curve);
  b = micros();
  timealice = microsecondsToClockCycles(b - a);

  a = micros();
  
  blake.reset(sizeof(Alice.authID));
  blake.update(Alice.tempPub, sizeof(Alice.tempPub));
  blake.finalize(Alice.authID, sizeof(Alice.authID));

  uECC_make_key(Bob.tempPub, Bob.tempPriv, curve);
  blake.reset(sizeof(Bob.authID));
  blake.update(Bob.tempPub, sizeof(Bob.tempPub));
  blake.finalize(Bob.authID, sizeof(Bob.authID));
  b = micros();
  
  timebob = microsecondsToClockCycles(b - a);

  a = micros();
  ECHMQV_get_shared_Secret(&Alice, Bob.publicKey, Bob.tempPub, curve);
  b = micros();
  timealice = timealice + microsecondsToClockCycles(b - a);
  
  a = micros();
  uECC_shared_secret2(Alice.publicKey, Alice.authID, Bob.sharedSecret, curve);
  EllipticAdd(Bob.sharedSecret, Alice.tempPub, Bob.sharedSecret, curve);

  modularMultAdd(Bob.authID, Bob.privateKey, Bob.tempPriv, Bob.tempPriv, curve);
  uECC_shared_secret2(Bob.sharedSecret, Bob.tempPriv, Bob.sharedSecret, curve);
  b = micros();
  timebob = timebob + microsecondsToClockCycles(b - a);

  Serial.print("ECHMQV (Alice) in: "); Serial.println(timealice);
  Serial.print("ECHMQV (Bob) in: "); Serial.println(timebob);

  if (memcmp(Alice.sharedSecret, Bob.sharedSecret, 24) != 0) {
    Serial.print("Gemeinsamer Schlüssel verschieden\n");
  } else {
    Serial.print("Gemeinsamer Schlüssel identisch\n");
  }

}
