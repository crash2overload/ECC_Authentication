#include <asm/types.h>
#include <gmp.h>

/****************** Tpyes *******************************/

typedef struct {
	mpz_t x0;
}Fp;

/***************** Funktions in Fp **********************/

void Fp_init(Fp *A);
void Fp_clear(Fp *A);
void Fp_print(Fp *A);
void Fp_inv(Fp *res, Fp *A);
void Fp_set_ui(Fp *A, unsigned int value);
void Fp_copy(Fp *dest, Fp *src);
void Fp_copy_ne(Fp *res,Fp *A);
void Fp_mul(Fp *res, Fp *A, Fp *B);
void Fp_mul_u(Fp *res, Fp *A, unsigned long int B);
void Fp_mul_mpz(Fp *res, Fp *A, mpz_t B);
void Fp_add(Fp *res, Fp *A, Fp *B);
void Fp_sub(Fp *res, Fp *A, Fp *B);
void Fp_exp(Fp *res, Fp *A, mpz_t sk);
void Fp_get_random(Fp *res);
int  Fp_cmp_zero(Fp *A);
int  Fp_cmp_one(Fp *A);
mpz_t prime;
