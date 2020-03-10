#include <gmp.h>

void init_params();
void clear_params();

mpz_t u; // 
mpz_t p; //prime
mpz_t n; //order
mpz_t t; //trace
int b; // Coefficient y^2 = x^3 + b
mpz_t zeta;
mpz_t zeta0;
mpz_t ht; //Cofactor of twist oder = ht*n
Fp2 Fp2_0;
Fp2 Fp2_1;
Fp2 Fp2_i;
Fp2 bt;
