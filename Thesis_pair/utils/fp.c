#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/fp.h"
#include "../include/curves.h"

/***************** Funktions in Fp **********************/

#define prec	192

void Fp_init(Fp *A){
	mpz_init(A->x0);
}
void Fp_clear(Fp *A){
	mpz_clear(A->x0);
}

void Fp_print(Fp *A){
	gmp_printf("%Zd\n", A->x0);
}

void Fp_inv(Fp* res, Fp *A){
	 mpz_invert(res->x0, A->x0, prime);
	
	//Fp_copy(res, A);
	//Fp_inv_core(res->x0);
}

void Fp_set_ui(Fp *A, unsigned int value){
	mpz_set_ui(A->x0, value);
}

void Fp_copy(Fp *dest, Fp *src){
	mpz_set(dest->x0, src->x0);
}

void Fp_copy_ne(Fp *res,Fp *A){
	mpz_sub(res->x0, prime, A->x0);
    mpz_mod(res->x0 ,res->x0, prime);
}

void Fp_mul(Fp *res, Fp *A, Fp *B){
	mpz_mul(res->x0, A->x0, B->x0);
    mpz_mod(res->x0, res->x0, prime);
}

void Fp_mul_u(Fp *res, Fp *A, unsigned long int B){
	mpz_mul_ui(res->x0, A->x0, B);
    mpz_mod(res->x0, res->x0, prime);
	
	//Fp_mult_core(res->x0, A->x0, B);
}

void Fp_mul_mpz(Fp *res, Fp *A, mpz_t B){
	mpz_mul(res->x0, A->x0, B);
    mpz_mod(res->x0, res->x0, prime);
}

void Fp_add(Fp *res, Fp *A, Fp *B){
	mpz_add(res->x0, A->x0, B->x0);
    mpz_mod(res->x0, res->x0, prime);
	
	//Fp_add_core(res->x0, A->x0, B->x0);
}

void Fp_sub(Fp *res, Fp *A, Fp *B){
	mpz_sub(res->x0, A->x0, B->x0);
    mpz_mod(res->x0, res->x0, prime);
	
	//Fp_sub_core(res->x0, A->x0, B->x0);
}

void Fp_exp(Fp *res, Fp *A, mpz_t sk){
	int i,length;
    length=(int)mpz_sizeinbase(sk,2);
    char binary[length];
    mpz_get_str(binary,2,sk);
    Fp tmp;
    Fp_init(&tmp);
    
    Fp_copy(&tmp, A);
    
    for(i=1; binary[i]!='\0'; i++){
        Fp_mul(&tmp, &tmp, &tmp);
        if(binary[i]=='1'){
            Fp_mul(&tmp, A, &tmp);
        }
    }
    Fp_copy(res, &tmp);
    
    Fp_clear(&tmp);
	
	//Fp_exp_core(res->x0, A->x0, sk->x0);
}

void Fp_get_random(Fp *res){
	gmp_randstate_t state;
	gmp_randinit_default(state);
	gmp_randseed_ui(state,1111111);
	mpz_urandomm(res->x0, state, prime);
	
	//make_random_Element(res->x0, prec);
}

int  Fp_cmp_zero(Fp *A){
    if(mpz_cmp_ui(A->x0,0)==0){
        return 0;
    }
    return 1;
}

int  Fp_cmp_one(Fp *A){
    if(mpz_cmp_ui(A->x0,1)==0){
        return 0;
    }
    return 1;
}



