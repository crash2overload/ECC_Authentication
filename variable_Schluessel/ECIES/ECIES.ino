#include <aes.hpp>
#include <uECC_vli.h>
#include <uECC.h>
#include <BLAKE2s.h>

BLAKE2s blake;

struct Node{
  uint8_t *privateKey;
  uint8_t *publicKey;
  uint8_t *tempPriv;
  uint8_t *tempPub;
  uint8_t *ID;
  uint8_t *sharedSecret;
  uint8_t keyMac[16] = {0};
  uint8_t keyEnc[16] = {0};
  uint8_t IV[16] = {0};
  uint8_t tag[16] = {0};
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

void ECIES_get_Keys(struct Node * node, uint8_t * publicKey, const struct uECC_Curve_t * curve){
  uECC_shared_secret2(publicKey, node->privateKey, node->sharedSecret, curve);
  
  blake.reset(uECC_curve_private_key_size(curve));
  blake.update(node->sharedSecret, uECC_curve_public_key_size(curve));
  blake.finalize(node->ID, uECC_curve_private_key_size(curve));
  
  memcpy(node->keyEnc, node->ID, 16);
  memcpy(node->keyMac, node->ID + 16, 16);
}

void ECIES_get_Tag(struct Node * node, uint8_t * message){
  blake.resetHMAC(node->keyMac, 16);
  blake.update(message, 32);
  blake.finalizeHMAC(node->keyMac, 16, node->tag, 16);
}

void setup() {
  Serial.begin(115200);
  Serial.print("ECIES\n");
  uECC_set_rng(&RNG2);

}

void loop() {
  const struct uECC_Curve_t * curve = uECC_secp192r1();

  Node Alice;
  Node Bob;
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

  uint8_t message[32] = {0};

  struct AES_ctx ctx;
  long Time_start, Time_end, timealice, timebob;

  uECC_make_key(Bob.publicKey, Bob.privateKey, curve);

  Time_start = micros();
  uECC_make_key(Alice.publicKey, Alice.privateKey, curve);
  
  ECIES_get_Keys(&Alice, Bob.publicKey, curve);
  ECIES_get_Tag(&Alice, message);

  Time_end = micros();
  timealice = microsecondsToClockCycles(Time_end - Time_start);

  AES_init_ctx_iv(&ctx, Alice.keyEnc, Alice.IV);
  AES_CTR_xcrypt_buffer(&ctx, message, sizeof(message));

  Time_start = micros();
  uECC_shared_secret2(Alice.publicKey, Bob.privateKey, Bob.sharedSecret, curve);

  ECIES_get_Keys(&Bob, Alice.publicKey, curve);

  AES_init_ctx_iv(&ctx, Bob.keyEnc, Bob.IV);
  AES_CTR_xcrypt_buffer(&ctx, message, sizeof(message));

  ECIES_get_Tag(&Bob, message);

  Time_end = micros();
  timebob = microsecondsToClockCycles(Time_end - Time_start);

  Serial.print("ECIES Alice in: "); Serial.println(timealice);
  Serial.print("ECIES Bob in: "); Serial.println(timebob);
  
  if (memcmp(Bob.tag, Alice.tag, 16) != 0) {
    Serial.print("Nachricht ist nicht authentisiert!\n");
  } else {
    Serial.print("Nachricht ist authentisiert\n");
  }
  
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
