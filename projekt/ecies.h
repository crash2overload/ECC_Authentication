#include <aes.hpp>

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


void ecies(const struct uECC_Curve_t * curve){

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

void ecies_bpv(const struct uECC_Curve_t * curve){

  uint8_t privateCA[24];
  uint8_t publicCA[48];

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

  uECC_make_key(publicCA, privateCA, curve);
  uECC_make_key(Bob.publicKey, Bob.privateKey, curve);

  AQ_make_privateKey(&Bob, privateCA, curve);
  uECC_mult_add(Bob.publicKey, Bob.ID, publicCA, Alice.sharedSecret, curve);

  Time_start = micros();
  getTempKeys(Alice.publicKey, Alice.privateKey, curve);
  
  ECIES_get_Keys(&Alice, Alice.sharedSecret, curve);
  ECIES_get_Tag(&Alice, message);  
  
  AES_init_ctx_iv(&ctx, Alice.keyEnc, Alice.IV);
  AES_CTR_xcrypt_buffer(&ctx, message, sizeof(message));

  Time_end = micros();
  timealice = (Time_end - Time_start);

  Time_start = micros();

  ECIES_get_Keys(&Bob, Alice.publicKey, curve);

  AES_init_ctx_iv(&ctx, Bob.keyEnc, Bob.IV);
  AES_CTR_xcrypt_buffer(&ctx, message, sizeof(message));

  ECIES_get_Tag(&Bob, message);

  Time_end = micros();
  timebob = microsecondsToClockCycles(Time_end - Time_start);


  Serial.print("Alice in: "); Serial.println(microsecondsToClockCycles(timealice));
  Serial.print("Bob in: "); Serial.println(timebob);

  if (memcmp(Bob.tag, Alice.tag, 16) != 0) 
    Serial.print("Nachricht nicht authentifiziert!\n");
  else 
    Serial.print("Nachricht ist authentifiziert\n");
    
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

void ecies_dbpv(const uECC_Curve_t * curve) {
  Node Alice;
  Node Bob;
 
  uint8_t message[32] = {0};

  uint8_t privateCA[24] = {0xB6, 0xE, 0x87, 0xB8, 0xDB, 0x7F, 0xB4, 0x3C, 0xBB, 0xDE, 0x1E, 0x1E, 0xCC, 0xFE, 0x44, 0x1, 0x26, 0xD4, 0xBB, 0xEE, 0xE8, 0x70, 0x18, 0x3E};
  uint8_t publicCA[48] = {0x9F, 0xD2, 0x62, 0xED, 0x71, 0x19, 0xEA, 0xF4, 0x64, 0x25, 0xCF, 0x22, 0x34, 0x7C, 0x90, 0xBA, 0xC6, 0x92, 0x24, 0x31, 0xBC, 0x9, 0x1E, 0x56, 0x55, 0x39, 0xC8, 0xAE, 0xBF, 0x7A, 0x79, 0x8B, 0xA3, 0xF2, 0xE5, 0x39, 0x5A, 0x48, 0xC9, 0x27, 0x48, 0x96, 0xEC, 0x4F, 0x68, 0x9C, 0xDB, 0xF9};


  struct AES_ctx ctx;
  long Time_start, Time_end, timealice, timebob;

  AQ_make_privateKey(&Bob, privateCA, curve);


  Time_start = micros();
  getTempKeysDBPV(Alice.publicKey, Alice.sharedSecret, Alice.privateKey, curve);
  ECIES_get_Keys(&Alice, Alice.sharedSecret, curve);
  ECIES_get_Tag(&Alice, message);  
  
  AES_init_ctx_iv(&ctx, Alice.keyEnc, Alice.IV);
  AES_CTR_xcrypt_buffer(&ctx, message, sizeof(message));

  Time_end = micros();
  timealice = (Time_end - Time_start);

  Time_start = micros();
  uECC_shared_secret2(Alice.publicKey, Bob.privateKey, Bob.sharedSecret, curve);
  ECIES_get_Keys(&Bob, Alice.sharedSecret, curve);
  
  AES_init_ctx_iv(&ctx, Bob.keyEnc, Bob.IV);
  AES_CTR_xcrypt_buffer(&ctx, message, sizeof(message));

  ECIES_get_Tag(&Bob, message);

  Time_end = micros();
  timebob = microsecondsToClockCycles(Time_end - Time_start);

  Serial.print("Alice in: "); Serial.println(microsecondsToClockCycles(timealice));
  Serial.print("Bob in: "); Serial.println(timebob);
  
  if (memcmp(Bob.tag, Alice.tag, 16) != 0) {
    Serial.print("Nachricht nicht authentifiziert!\n");
  } else {
    Serial.print("Nachricht authentifiziert\n");
  }

}
