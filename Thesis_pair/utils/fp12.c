#include "../include/fp12.h"

void Fp12_init(Fp12 *A){
	Fp6_init(&A->x0);
	Fp6_init(&A->x1);
}
void Fp12_clear(Fp12 *A){
	Fp6_clear(&A->x0);
	Fp6_clear(&A->x1);
}

void Fp12_print(Fp12 *A){
	Fp6_print(&A->x0);
	Fp6_print(&A->x1);
}

void Fp12_inv(Fp12 *res, Fp12 *A){
    Fp6 c_x0,c_x1,t0,t1;
    Fp6_init(&c_x0);
    Fp6_init(&c_x1);
    Fp6_init(&t0);
    Fp6_init(&t1);
    
    Fp6_copy(&c_x0, &A->x0);
    Fp6_set_ne(&c_x1, &A->x1);
    
    Fp6_mul(&t0, &c_x0, &A->x0);
    Fp6_mul(&t1, &c_x1, &A->x1);
    Fp6_mul_basis(&t1, &t1);
    Fp6_add(&t0, &t0, &t1);
    Fp6_inv(&t0, &t0);
    Fp6_mul(&res->x0, &c_x0, &t0);
    Fp6_mul(&res->x1, &c_x1, &t0);
    
    Fp6_clear(&c_x0);
    Fp6_clear(&c_x1);
    Fp6_clear(&t0);
    Fp6_clear(&t1);
}


void Fp12_copy(Fp12 *dest, Fp12 *src){
	Fp6_copy(&dest->x0, &src->x0);
	Fp6_copy(&dest->x1, &src->x1);
}

void Fp12_mul(Fp12 *res, Fp12 *A, Fp12 *B){
	Fp6 temp1, temp2;
	Fp6_init(&temp1);
	Fp6_init(&temp2);
	
	Fp6_mul(&temp2, &A->x1, &B->x1);//b*d
    Fp6_add(&temp1, &A->x0, &A->x1);//a+b
    Fp6_add(&res->x1, &B->x0, &B->x1);//c+d
    Fp6_mul(&res->x1, &temp1, &res->x1);//(a+b)(c+d)
    Fp6_mul(&temp1, &A->x0, &B->x0);//a*c
    //x0
    Fp6_mul_basis(&res->x0, &temp2);//b*d*v
    Fp6_add(&res->x0, &res->x0, &temp1);//a*c+b*d*v
    //x1
    Fp6_sub(&res->x1, &res->x1, &temp1);
    Fp6_sub(&res->x1, &res->x1, &temp2);
    
    Fp6_clear(&temp1);
    Fp6_clear(&temp2);
}

void Fp12_sqr(Fp12 *ANS,Fp12 *A){
    Fp6 tmp1,tmp2,tmp3;
	Fp6_init(&tmp1);
	Fp6_init(&tmp2);
	Fp6_init(&tmp3);
	
	Fp6_add(&tmp1,&A->x0,&A->x1);
	Fp6_mul_basis(&tmp2,&A->x1);
	Fp6_add(&tmp2,&tmp2,&A->x0);
	Fp6_mul(&tmp3,&A->x0,&A->x1);
	
	//x0
	Fp6_mul(&ANS->x0,&tmp1,&tmp2);
	Fp6_sub(&ANS->x0,&ANS->x0,&tmp3);
	Fp6_mul_basis(&tmp1,&tmp3);
	Fp6_sub(&ANS->x0,&ANS->x0,&tmp1);
	//x1
	Fp6_add(&ANS->x1,&tmp3,&tmp3);
	
	Fp6_clear(&tmp1);
	Fp6_clear(&tmp2);
	Fp6_clear(&tmp3);
}


void Fp12_add(Fp12 *res, Fp12 *A, Fp12 *B){
	Fp6_add(&res->x0, &A->x0, &B->x0);
	Fp6_add(&res->x1, &A->x1, &B->x1);
}

void Fp12_sub(Fp12 *res, Fp12 *A, Fp12 *B){
	Fp6_sub(&res->x0, &A->x0, &B->x0);
	Fp6_sub(&res->x1, &A->x1, &B->x1);
}

void Fp12_exp(Fp12 *res, Fp12 *A, mpz_t sk){
	 int i,length;
    length=(int)mpz_sizeinbase(sk,2);
    char binary[length];
    mpz_get_str(binary,2,sk);
    Fp12 tmp;
    Fp12_init(&tmp);
    Fp12_copy(&tmp,A);
    
    for(i=1; binary[i]!='\0'; i++){
        Fp12_sqr(&tmp,&tmp);
        if(binary[i]=='1'){
            Fp12_mul(&tmp,A,&tmp);
        }
    }
    
    Fp12_copy(res,&tmp);
    Fp12_clear(&tmp);
}
