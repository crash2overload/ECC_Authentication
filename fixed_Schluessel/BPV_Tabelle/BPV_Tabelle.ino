#include <uECC_vli.h>
#include <uECC.h>
#include <types.h>

#include <BLAKE2s.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <time.h>
//#include "keys.h"


static int RNG2(uint8_t *dest, unsigned size) {

  while (size) {
    uint8_t val = 0;
    val = rand() % 256;
    *dest = val;
    ++dest;
    --size;
  }
  // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
  return 1;
}

BLAKE2s blake;
  
void setup() {
  Serial.begin(115200);
  Serial.print("Arazi\n");
  time_t t;
  srand((unsigned) time(&t));
  uECC_set_rng(&RNG2);
  
  const struct uECC_Curve_t * curve = uECC_secp192r1();

  //uint8_t publicAlice1[48];
  //uint8_t privateAlice1[24];

  
  uint8_t *publicBob1 = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  uint8_t *privateBob1 = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  
  uint8_t *publicCA = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  uint8_t *privateCA = (uint8_t*)malloc(uECC_curve_private_key_size(curve));

  Serial.println("\npublic Bob:");
  uECC_make_key(publicBob1, privateBob1, curve);
  for (int i = 0; i < uECC_curve_public_key_size(curve); i++)
  {
    Serial.print("0x"); Serial.print(publicBob1[i], HEX); Serial.print(", ");
  }
  Serial.println("\nprivate Bob:");
  
  for (int i = 0; i < uECC_curve_private_key_size(curve); i++)
  {
    Serial.print("0x"); Serial.print(privateBob1[i], HEX); Serial.print(", ");
  }
  
  uECC_make_key(publicCA, privateCA, curve);

  Serial.println("\npublic CA:");
  
  for (int i = 0; i < uECC_curve_public_key_size(curve); i++)
  {
    Serial.print("0x"); Serial.print(publicCA[i], HEX); Serial.print(", ");
  }
  Serial.println("\nprivate CA:");
  
  for (int i = 0; i < uECC_curve_private_key_size(curve); i++)
  {
    Serial.print("0x"); Serial.print(privateCA[i], HEX); Serial.print(", ");
  }
  
  uint8_t *publicAlice1 = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  uint8_t *privateAlice1 = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  
  uint8_t *pointAlice1 = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  uint8_t *pointBob1 = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  uint8_t *hash = (uint8_t*)malloc(uECC_curve_private_key_size(curve));

  blake.reset(uECC_curve_private_key_size(curve));
  blake.update(publicBob1, uECC_curve_public_key_size(curve));
  blake.finalize(hash, uECC_curve_private_key_size(curve));

  uECC_shared_secret2(publicBob1, hash, pointAlice1, curve);
  EllipticAdd(pointAlice1, publicCA, pointAlice1, curve);
  Serial.println("\nTable :");
  for (int i = 0; i < 160; i++)
  {
      uECC_make_key(publicAlice1, privateAlice1, curve);
      for (int j = 0; j < uECC_curve_private_key_size(curve); ++j) {
      Serial.print("0x"); Serial.print(privateAlice1[j], HEX); Serial.print(", ");
      }
      uECC_shared_secret2(pointAlice1, privateAlice1, pointBob1, curve);
      for (int j = 0; j < uECC_curve_public_key_size(curve); ++j) {
      Serial.print("0x"); Serial.print(pointBob1[j], HEX); Serial.print(", ");
      }
      Serial.print("\n");
  }
  Serial.print("\nfertig");
}

void loop() {

  
}
