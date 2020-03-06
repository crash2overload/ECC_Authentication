#include "fp6.h"

/*********************** Types *************************/

typedef struct {
	Fp6 x0, x1;
}Fp12;


/***************** Funktions in Fp12 **********************/

void Fp12_init(Fp12 *A);
void Fp12_clear(Fp12 *A);
void Fp12_print(Fp12 *A);
void Fp12_inv(Fp12 *res, Fp12 *A);
void Fp12_copy(Fp12 *dest, Fp12 *src);
void Fp12_mul(Fp12 *res, Fp12 *A, Fp12 *B);
void Fp12_sqr(Fp12 *ANS,Fp12 *A);
void Fp12_add(Fp12 *res, Fp12 *A, Fp12 *B);
void Fp12_sub(Fp12 *res, Fp12 *A, Fp12 *B);
void Fp12_exp(Fp12 *res, Fp12 *A, mpz_t sk);
