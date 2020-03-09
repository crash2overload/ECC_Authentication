#include "efp.h"

void Pseudo_8_sparse_mapping(EFp *P, EFp2 *Q, Fp *L);
void Pseudo_8_sparse_mul(Fp12 *res, Fp12 *A, Fp12 *B);
void ff_ltt(Fp12 *f, EFp2 *T, EFp2 *Q, EFp *P);
//void f_ltq(Fp12 *f,EFp2 *T,EFp2 *Q,EFp *P,Fp *L);
void Miller_ate(Fp12 *res, EFp2 *Q, EFp *P);
void Final_exp(Fp12 *res);

Fp2 frobenius_constant[12][6];
