struct Node{
  uint8_t *privateKey;
  uint8_t *publicKey;
  uint8_t *tempPriv;
  uint8_t *tempPub;
  uint8_t *ID;
  uint8_t *sharedSecret;
};

void AQ_make_privateKey(struct Node * node, uint8_t * privateCA, const struct uECC_Curve_t * curve);
void AQ_shared_Secret(uint8_t * ID, uint8_t * publicKey, uint8_t * emphermalPublicKey,struct Node * node, uint8_t * publicCA, const struct uECC_Curve_t * curve);
