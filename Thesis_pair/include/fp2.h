#include "fp.h"

/********************* Types *****************************/

typedef struct {
	Fp x0, x1;
}Fp2;

/***************** Functions in Fp2 **********************/

void Fp2_init(Fp2 *A);
void Fp2_clear(Fp2 *A);
void Fp2_print(Fp2 *A);
void Fp2_copy(Fp2 *dest, Fp2 *src);
void Fp2_set_ui(Fp2 *ANS,__u32 UI);
void Fp2_copy_ne(Fp2 *res,Fp2 *A);
void Fp2_inv(Fp2 *res, Fp2 *A);
void Fp2_sqr(Fp2 *res, Fp2 *A);
void Fp2_mul(Fp2 *res, Fp2 *A, Fp2 *B);
void Fp2_mul_u(Fp2 *res, Fp2 *A, unsigned long int B);
void Fp2_mul_mpz(Fp2 *res, Fp2 *A, mpz_t B);
void Fp2_mul_basis(Fp2 *res, Fp2 *A);
void Fp2_add(Fp2 *res, Fp2 *A, Fp2 *B);
void Fp2_sub(Fp2 *res, Fp2 *A, Fp2 *B);
void Fp2_exp(Fp2 *res, Fp2 *A, mpz_t sk);
int  Fp2_legendre(Fp2 *A);
void Fp2_set_random(Fp2 *ANS);
void Fp2_sqrt(Fp2 *ANS,Fp2 *A);
