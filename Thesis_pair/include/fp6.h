#include "fp2.h"


/********************** Types ****************************/

typedef struct {
	Fp2 x0, x1, x2;
}Fp6;

/***************** Funktions in Fp6 **********************/

void Fp6_init(Fp6 *A);
void Fp6_clear(Fp6 *A);
void Fp6_print(Fp6 *A);
void Fp6_inv(Fp6 *res, Fp6 *A);
void Fp6_copy(Fp6 *dest, Fp6 *src);
void Fp6_set_ne(Fp6 *res, Fp6 *A);
void Fp6_mul(Fp6 *res, Fp6 *A, Fp6 *B);
void Fp6_mul_basis(Fp6 *res, Fp6 *A);
void Fp6_add(Fp6 *res, Fp6 *A, Fp6 *B);
void Fp6_sub(Fp6 *res, Fp6 *A, Fp6 *B);
void Fp6_exp(Fp6 *res, Fp6 *A, Fp6 *sk);
