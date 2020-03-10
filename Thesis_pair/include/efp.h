#include "fp12.h"
#include "params.h"

typedef struct {
	Fp x,y,z;
}EFp;

typedef struct {
	Fp2 x,y, z;
}EFp2;

typedef struct {
	Fp6 x,y;
}EFp6;

typedef struct {
	Fp12 x,y;
}EFp12;

void EFp_init(EFp *P);
void EFp_clear(EFp *P);
void EFp_print(EFp *P);
void EFp_copy(EFp *dest, EFp *src);
void EFp_set_generator(EFp *P);
void EFp_add(EFp *res, EFp *P, EFp *Q);
void EFp_dbb(EFp *res, EFp *P);
void EFp_mul(EFp *res, mpz_t sk, EFp *P);
void EFp_to_affine(EFp *P);

//void EFp2_init(EFp2 *P);
//void EFp2_set(EFp2 *ANS,EFp2 *A);
void EFp2_set_ui(EFp2 *ANS,unsigned long int UI);
void EFp2_set_mpz(EFp2 *ANS,mpz_t A);
void EFp2_set_neg(EFp2 *ANS,EFp2 *A);
//void EFp2_clear(EFp2 *P);
void EFp2_printf(EFp2 *P,char *str);
void EFp2_rational_point(EFp2 *P);
void EFp2_add(EFp2 *res, EFp2 *P, EFp2 *Q);
void EFp2_dbb(EFp2 *res, EFp2 *P);
void EFp2_mul(EFp2 *res, mpz_t sk, EFp2 *P);


void EFp2_init(EFp2 *P);
void EFp2_clear(EFp2 *P);
//void EFp2_print(EFp2 *P);
void EFp2_copy(EFp2 *dest, EFp2 *src);
void EFp2_set_generator(EFp2 *P);
