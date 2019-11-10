void ECHMQV_get_shared_Secret(struct Node * node, uint8_t * publicKey, uint8_t * emphermalPublicKey, const struct uECC_Curve_t * curve){
  
  uint8_t *authID2 = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  
  blake.reset();
  blake.update(node->tempPub, uECC_curve_public_key_size(curve));
  blake.finalize(node->authID, uECC_curve_private_key_size(curve));

  blake.reset(uECC_curve_private_key_size(curve));
  blake.update(emphermalPublicKey, uECC_curve_public_key_size(curve));
  blake.finalize(authID2, uECC_curve_private_key_size(curve));

  uECC_mult_add(publicKey, authID2, emphermalPublicKey, node->sharedSecret, curve);

  modularMultAdd(node->authID, node->privateKey, node->tempPriv, node->tempPriv, curve);
  uECC_shared_secret2(node->sharedSecret, node->tempPriv, node->sharedSecret, curve);

  free(authID2);
}

void ECHMQV_get_shared_Secret2(struct Node * node, uint8_t * emphermalPublicKey, const struct uECC_Curve_t * curve){
  
  uint8_t *authID2 = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  
  blake.reset();
  blake.update(node->tempPub, uECC_curve_public_key_size(curve));
  blake.finalize(node->authID, uECC_curve_private_key_size(curve));

  blake.reset(uECC_curve_private_key_size(curve));
  blake.update(emphermalPublicKey, uECC_curve_public_key_size(curve));
  blake.finalize(authID2, uECC_curve_private_key_size(curve));

  uECC_mult_add(node->sharedSecret, authID2, emphermalPublicKey, node->sharedSecret, curve);
  
  modularMultAdd(node->privateKey, node->authID, node->tempPriv, node->tempPriv, curve);

  uECC_shared_secret2(node->sharedSecret, node->tempPriv, node->sharedSecret, curve);
}

void echmqv(const uECC_Curve_t * curve) {
  Node Alice, Bob;
  Alice.publicKey = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  Alice.privateKey = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Alice.tempPub = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  Alice.tempPriv = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Alice.ID = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Alice.authID = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Alice.sharedSecret = (uint8_t*)malloc(uECC_curve_public_key_size(curve));

  Bob.publicKey = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  Bob.privateKey = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Bob.tempPub = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  Bob.tempPriv = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Bob.ID = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Bob.authID = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  Bob.sharedSecret = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  
  uint8_t *privateCA = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  uint8_t *publicCA = (uint8_t*)malloc(uECC_curve_public_key_size(curve));

  unsigned long a, b, timebob, timealice;

  uECC_make_key(publicCA, privateCA, curve);
  uECC_make_key(Bob.publicKey, Bob.privateKey, curve);
  uECC_make_key(Alice.publicKey, Alice.privateKey, curve);

  blake.reset(uECC_curve_public_key_size(curve));
  blake.update(Alice.publicKey, uECC_curve_public_key_size(curve));
  blake.finalize(Alice.ID, uECC_curve_private_key_size(curve));

  blake.reset(uECC_curve_private_key_size(curve));
  blake.update(Bob.publicKey, uECC_curve_public_key_size(curve));
  blake.finalize(Bob.ID, uECC_curve_private_key_size(curve));

  a = micros();
  uECC_make_key(Alice.tempPub, Alice.tempPriv, curve);
  b = micros();
  timealice = microsecondsToClockCycles(b - a);

  a = micros();
  
  blake.reset(uECC_curve_public_key_size(curve));
  blake.update(Alice.tempPub, uECC_curve_public_key_size(curve));
  blake.finalize(Alice.authID, uECC_curve_private_key_size(curve));

  uECC_make_key(Bob.tempPub, Bob.tempPriv, curve);
  blake.reset(uECC_curve_public_key_size(curve));
  blake.update(Bob.tempPub, uECC_curve_public_key_size(curve));
  blake.finalize(Bob.authID, uECC_curve_private_key_size(curve));
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

  free(privateCA);
  free(publicCA);
  free(Alice.publicKey);
  free(Alice.privateKey);
  free(Alice.tempPub);
  free(Alice.tempPriv);
  free(Alice.ID);
  free(Alice.authID);
  free(Alice.sharedSecret);
  free(Bob.privateKey);
  free(Bob.publicKey);
  free(Bob.tempPriv);
  free(Bob.tempPub);
  free(Bob.ID);
  free(Bob.authID);
  free(Bob.sharedSecret);
}

void echmqv_bpv(const struct uECC_Curve_t * curve){
  
  Node Alice, Bob;
  Alice.publicKey = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  Alice.privateKey = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Alice.tempPub = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  Alice.tempPriv = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Alice.ID = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Alice.authID = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Alice.sharedSecret = (uint8_t*)malloc(uECC_curve_public_key_size(curve));

  Bob.publicKey = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  Bob.privateKey = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Bob.tempPub = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  Bob.tempPriv = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Bob.ID = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  Bob.authID = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  Bob.sharedSecret = (uint8_t*)malloc(uECC_curve_public_key_size(curve));
  
  uint8_t *privateCA = (uint8_t*)malloc(uECC_curve_private_key_size(curve));
  uint8_t *publicCA = (uint8_t*)malloc(uECC_curve_public_key_size(curve));

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
  blake.reset(uECC_curve_private_key_size(curve));
  blake.update(Alice.tempPub, uECC_curve_public_key_size(curve));
  blake.finalize(Alice.authID, uECC_curve_private_key_size(curve));

  blake.reset(uECC_curve_private_key_size(curve));
  blake.update(Bob.tempPub, uECC_curve_public_key_size(curve));
  blake.finalize(Bob.authID, uECC_curve_private_key_size(curve));
    
  b = micros();
  timebob = timebob + microsecondsToClockCycles(b - a);

   a = micros();
  ECHMQV_get_shared_Secret2(&Alice, Bob.tempPub, curve);
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
  free(privateCA);
  free(publicCA);
  free(Alice.publicKey);
  free(Alice.privateKey);
  free(Alice.tempPub);
  free(Alice.tempPriv);
  free(Alice.ID);
  free(Alice.authID);
  free(Alice.sharedSecret);
  free(Bob.privateKey);
  free(Bob.publicKey);
  free(Bob.tempPriv);
  free(Bob.tempPub);
  free(Bob.ID);
  free(Bob.authID);
  free(Bob.sharedSecret);
}
