#include <aes.hpp>

void Signcryption_get_Keys(struct Node * node, uint8_t * publicKey, const struct uECC_Curve_t * curve){
  uECC_shared_secret2(publicKey, node->tempPriv, node->sharedSecret, curve);

  blake.reset(uECC_curve_private_key_size(curve));
  blake.update(node->sharedSecret, uECC_curve_public_key_size(curve));
  blake.finalize(node->ID, uECC_curve_private_key_size(curve));
  
  memcpy(node->keyEnc, node->ID, 16);
  memcpy(node->keyMac, node->ID + 16, 16);
}

void SigncryptionDBPV_get_Keys(struct Node * node, const struct uECC_Curve_t * curve){
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

void UnsigncryptionDBPV_get_Keys(struct Node * node, uint8_t * r, uint8_t * s, const struct uECC_Curve_t * curve){
  modularMult2(s, node->privateKey, node->s, curve);
  uECC_compute_public_key(r, node->tempPub, curve);
  
  EllipticAdd(node->sharedSecret, node->tempPub, node->sharedSecret, curve);
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


void signcryption(const uECC_Curve_t * curve) {
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

void signcryption_dbpv(const uECC_Curve_t * curve) {
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

  uint8_t privateCA[24] = {0xB6, 0xE, 0x87, 0xB8, 0xDB, 0x7F, 0xB4, 0x3C, 0xBB, 0xDE, 0x1E, 0x1E, 0xCC, 0xFE, 0x44, 0x1, 0x26, 0xD4, 0xBB, 0xEE, 0xE8, 0x70, 0x18, 0x3E};
  uint8_t publicCA[48] = {0x9F, 0xD2, 0x62, 0xED, 0x71, 0x19, 0xEA, 0xF4, 0x64, 0x25, 0xCF, 0x22, 0x34, 0x7C, 0x90, 0xBA, 0xC6, 0x92, 0x24, 0x31, 0xBC, 0x9, 0x1E, 0x56, 0x55, 0x39, 0xC8, 0xAE, 0xBF, 0x7A, 0x79, 0x8B, 0xA3, 0xF2, 0xE5, 0x39, 0x5A, 0x48, 0xC9, 0x27, 0x48, 0x96, 0xEC, 0x4F, 0x68, 0x9C, 0xDB, 0xF9};

  struct AES_ctx ctx;
  uint8_t message[32] = {0};
  unsigned long a, b, c, d;

  uECC_make_key(Alice.publicKey, Alice.privateKey, curve);
  
  AQ_make_privateKey(&Alice, privateCA, curve);
  uECC_mult_add(Alice.publicKey, Alice.ID, publicCA, Bob.sharedSecret, curve);
  
  AQ_make_privateKey(&Bob, privateCA, curve);
  
  a = micros();
  getTempKeys(Alice.sharedSecret, Alice.tempPriv, curve);
  SigncryptionDBPV_get_Keys(&Alice, curve);
  Signcryption_get_r(&Alice, message);
  
  AES_init_ctx_iv(&ctx, Alice.keyEnc, Alice.IV);
  AES_CTR_xcrypt_buffer(&ctx, message, sizeof(message));

  Signcryption_get_s(&Alice, curve);
  b = micros();
  unsigned long clockcycle;
  clockcycle = microsecondsToClockCycles(b-a);
  Serial.print("Signcryption (Alice) in: "); Serial.println(clockcycle);



  c = micros();
  UnsigncryptionDBPV_get_Keys(&Bob, Alice.tag, Alice.s, curve);
   
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
