#include "util.h"
#include "aq.h"
#include "echmqv.h"
#include "ecies.h"
#include "signcryption.h"
#include <uECC_vli.h>
#include <uECC.h>



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

void setup() {
  Serial.begin(115200);
  Serial.print("Optimized Suite\n");
  uECC_set_rng(&RNG2);
}

int nummer = 0;

void loop() {
  const struct uECC_Curve_t * curve;
  Serial.println("1 160 bit");
  Serial.println("2 192 bit");
  Serial.println("3 224 bit");
  Serial.println("4 256 bit");
  Serial.print("Bitte Schlüssellänge eingeben: ");
  while (Serial.available() == 0 || nummer == 0)
    nummer = Serial.parseInt();

  switch (nummer) {
    case 1: curve = uECC_secp160r1(); table = TabBPV160; break;
    case 2: curve = uECC_secp192r1(); table = TabBPV; break;
    case 3: curve = uECC_secp224r1(); table = TabBPV224; break;
    case 4: curve = uECC_secp256r1(); table = TabBPV256; break;
    default: Serial.println("Eingabe nicht akzeptiert!!!");
  }
  Serial.println(nummer);
  nummer = 0;
  
  Serial.println("");
  Serial.println("1 AQ");
  Serial.println("2 AQ-BPV");
  Serial.println("3 ECHMQV");
  Serial.println("4 ECHMQV-BPV");
  Serial.println("5 ECIES");
  Serial.println("6 ECIES-BPV");
  Serial.println("7 ECIES-DBPV");
  Serial.println("8 Signcryption");
  Serial.println("9 Signcryption-DBPV");
  Serial.print("Bitte Nummer des Verfahrens eingeben: ");
  while (Serial.available() == 0 || nummer == 0)
    nummer = Serial.parseInt();
  Serial.println(nummer);
  Serial.println("");
  Serial.println("-----------------------------------------------------");
  Serial.println("");
  switch (nummer) {
    case 1: aq(curve); break;
    case 2: aq_bpv(curve); break;
    case 3: echmqv(curve); break;
    case 4: echmqv_bpv(curve); break;
    case 5: ecies(curve); break;
    case 6: ecies_bpv(curve); break;
    case 7: ecies_dbpv(curve); break;
    case 8: signcryption(curve); break;
    case 9: break;
    default: Serial.println("Eingabe nicht akzeptiert!!!");
  }
  Serial.println("");
  Serial.println("-----------------------------------------------------");
  Serial.println("");
  nummer = 0;

}
