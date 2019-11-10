
struct Node{
  uint8_t privateKey[24];
  uint8_t publicKey[48];
  uint8_t tempPriv[24];
  uint8_t tempPub[48];
  uint8_t ID[24];
  uint8_t authID[24];
  uint8_t sharedSecret[48];
};

void ECHMQV_get_shared_Secret(struct Node * node, uint8_t * publicKey, uint8_t * emphermalPublicKey, const struct uECC_Curve_t * curve);
