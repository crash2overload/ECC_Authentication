#include <uECC_vli.h>
#include <uECC.h>
#include <BLAKE2s.h>
#include "keys_192.h"
//#include "keys_160.h"
//#include "keys_224.h"
//#include "keys_256.h"

BLAKE2s blake;

struct Node{
  uint8_t *privateKey;
  uint8_t *publicKey;
  uint8_t *tempPriv;
  uint8_t *tempPub;
  uint8_t *ID;
  uint8_t *authID;
  uint8_t *sharedSecret;
};


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
