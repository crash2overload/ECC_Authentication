#include <aes.hpp>
#include <uECC_vli.h>
#include <uECC.h>
#include <BLAKE2s.h>

BLAKE2s blake;

struct Node{
  uint8_t privateKey[24];
  uint8_t publicKey[48];
  uint8_t tempPriv[24];
  uint8_t tempPub[48];
  uint8_t ID[24];
  uint8_t sharedSecret[48];
  uint8_t keyMac[16] = {0};
  uint8_t keyEnc[16] = {0};
  uint8_t IV[16] = {0};
  uint8_t tag[24] = {0};
  uint8_t s[24] = {0};
};

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

void Signcryption_get_Keys(struct Node * node, uint8_t * publicKey, const struct uECC_Curve_t * curve){
  uECC_shared_secret2(publicKey, node->tempPriv, node->sharedSecret, curve);

  blake.reset(uECC_curve_private_key_size(curve));
  blake.update(node->sharedSecret, uECC_curve_public_key_size(curve));
  blake.finalize(node->ID, uECC_curve_private_key_size(curve));
  
  memcpy(node->keyEnc, node->ID, 16);
  memcpy(node->keyMac, node->ID + 16, 16);
}

void Unsigncryption_get_Keys(struct Node * node, uint8_t * r, uint8_t * s, uint8_t * publicKey, const struct uECC_Curve_t * curve){
  modularMult2(s, node->privateKey, node->s, curve);
  uECC_compute_public_key(r, node->sharedSecret, curve);
  
  EllipticAdd(node->sharedSecret, publicKey, node->sharedSecret, curve);
  uECC_shared_secret2(node->sharedSecret, node->s, node->sharedSecret, curve);
    
  blake.reset(uECC_curve_private_key_size(curve));
  blake.update(node->sharedSecret, uECC_curve_public_key_size(curve));
  blake.finalize(node->ID, uECC_curve_private_key_size(curve));
  
  memcpy(node->keyEnc, node->ID, 16);
  memcpy(node->keyMac, node->ID + 16, 16);
}

void Signcryption_get_r(struct Node * node, uint8_t * message){
  blake.resetHMAC(node->keyMac, 16);
  blake.update(message, 32);
  blake.finalizeHMAC(node->keyMac, 16, node->tag, 24);
}

void Signcryption_get_s(struct Node * node, const struct uECC_Curve_t * curve){
  modularAdd2(node->privateKey, node->tag, node->s, curve);
  modularInv2(node->s, node->s, curve);
  modularMult2(node->tempPriv, node->s, node->s, curve);
}

void setup() {
  Serial.begin(115200);
  Serial.print("Signcryption\n");
  uECC_set_rng(&RNG2);

}

void loop() {
  const struct uECC_Curve_t * curve = uECC_secp192r1();

  Node Alice;
  Node Bob;

  uint8_t message[32] = {0};

  struct AES_ctx ctx;

  unsigned long a, b, c, d;

  uECC_make_key(Bob.publicKey, Bob.privateKey, curve);
  uECC_make_key(Alice.publicKey, Alice.privateKey, curve);


  a = micros();
  uECC_make_private_key(Alice.tempPriv, curve);
  Signcryption_get_Keys(&Alice, Bob.publicKey, curve);
  Signcryption_get_r(&Alice, message);
  
  AES_init_ctx_iv(&ctx, Alice.keyEnc, Alice.IV);
  AES_CTR_xcrypt_buffer(&ctx, message, sizeof(message));

  Signcryption_get_s(&Alice, curve);
  b = micros();
  unsigned long clockcycle;
  clockcycle = microsecondsToClockCycles(b-a);
  Serial.print("Signcryption (Alice) in: "); Serial.println(clockcycle);

  c = micros();
  Unsigncryption_get_Keys(&Bob, Alice.tag, Alice.s, Alice.publicKey, curve);
  AES_init_ctx_iv(&ctx, Bob.keyEnc, Bob.IV);
  AES_CTR_xcrypt_buffer(&ctx, message, sizeof(message));

  Signcryption_get_r(&Bob, message);

  if (memcmp(Bob.tag, Alice.tag, 16) != 0) {
    Serial.print("Message IS NOT Authenticated!\n");
  } else {
    Serial.print("Message is Authenticated\n");
  }
  d = micros();
  unsigned long clockcycle2;
  clockcycle2 = microsecondsToClockCycles(d-c);
  Serial.print("Signcryption (Bob) in: "); Serial.println(clockcycle2);  

}
