#include <uECC_vli.h>
#include <uECC.h>
#include <BLAKE2s.h>
#include "keys_192.h"
//#include "keys_160.h"
//#include "keys_224.h"
//#include "keys_256.h"
#include "Arazi.h"

BLAKE2s blake;

static int RNG2(uint8_t *dest, unsigned size) {

  while (size) {
    uint8_t val = 0;
    val = random(255);
    *dest = val;
    ++dest;
    --size;
  }
  return 1;
}

void AQ_make_privateKey(struct Node * node, uint8_t * privateCA, const struct uECC_Curve_t * curve){
   /* hash = h(ID,U) */
  blake.reset();
  blake.update(node->publicKey, uECC_curve_public_key_size(curve));
  blake.finalize(node->ID, uECC_curve_private_key_size(curve));  
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
  Serial.print("AQ und BPV\n");
  uECC_set_rng(&RNG2);
}

void getTempKeys(uint8_t * publicKey, uint8_t * privateKey, const uECC_Curve_t * curve)
{
  uint8_t *tempPriv = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  uint8_t *tempPub = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  int sumKeys = uECC_curve_private_key_size(curve) + uECC_curve_public_key_size(curve);
  
  long randNummer = random(160);
  for (int i = 0; i < uECC_curve_private_key_size(curve); i++)
  {
    tempPriv[i] = pgm_read_word_near(TabBPV + sumKeys * randNummer + i);
  }

  for (int i = uECC_curve_private_key_size(curve); i < sumKeys; i++)
  {
    tempPub[i - uECC_curve_private_key_size(curve)] = pgm_read_word_near(TabBPV + sumKeys * randNummer + i);
  }
  for (int j = 0; j < 7; j++)
  {
   randNummer = random(160);
    for (int i = 0; i < uECC_curve_private_key_size(curve); i++)
    {
      privateKey[i] = pgm_read_word_near(TabBPV + sumKeys * randNummer + i);
    }

    for (int i = uECC_curve_private_key_size(curve); i < sumKeys; i++)
    {
      publicKey[i - uECC_curve_private_key_size(curve)] = pgm_read_word_near(TabBPV + sumKeys * randNummer + i);
    }

  EllipticAdd(publicKey, tempPub, publicKey, curve);
  modularAdd2(privateKey, tempPriv, privateKey, curve);
  }
  free(tempPriv);
  free(tempPub);
  
}

void loop() {
  const struct uECC_Curve_t * curve = uECC_secp192r1();
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
  getTempKeys(Alice.tempPub, Alice.tempPriv, curve); 
  b = micros();
  timealice = timealice + (b - a);

    
  a = micros();
  getTempKeys(Bob.tempPub, Bob.tempPriv, curve);
  b = micros();
  timebob = timebob + (b - a);

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
