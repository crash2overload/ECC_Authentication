
struct Node{
  uint8_t *privateKey;
  uint8_t *publicKey;
  uint8_t *tempPriv;
  uint8_t *tempPub;
  uint8_t *ID;
  uint8_t *authID;
  uint8_t *sharedSecret;
};

void ECHMQV_get_shared_Secret(struct Node * node, uint8_t * publicKey, uint8_t * emphermalPublicKey, const struct uECC_Curve_t * curve);
