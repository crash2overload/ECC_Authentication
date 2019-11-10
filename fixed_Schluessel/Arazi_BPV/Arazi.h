struct Node{
  uint8_t privateKey[24];
  uint8_t publicKey[48];
  uint8_t tempPriv[24];
  uint8_t tempPub[48];
  uint8_t ID[24];
  uint8_t sharedSecret[48];
};

void AQ_make_privateKey(struct Node * node, uint8_t * privateCA, const struct uECC_Curve_t * curve);
void AQ_shared_Secret(uint8_t * ID, uint8_t * publicKey, uint8_t * emphermalPublicKey,struct Node * node, uint8_t * publicCA, const struct uECC_Curve_t * curve);
