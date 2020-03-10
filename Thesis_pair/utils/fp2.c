#include "../include/fp2.h"
#include <stdio.h>

void Fp2_init(Fp2 *A){
	Fp_init(&A->x0);
	Fp_init(&A->x1);
}
void Fp2_clear(Fp2 *A){
	Fp_clear(&A->x0);
	Fp_clear(&A->x1);
}

void Fp2_print(Fp2 *A){
	Fp_print(&A->x0);
	printf(",");
	Fp_print(&A->x1);
}

void Fp2_copy(Fp2 *dest, Fp2 *src){
	Fp_copy(&dest->x0, &src->x0);
	Fp_copy(&dest->x1, &src->x1);
}

void Fp2_set_ui(Fp2 *res, unsigned int value){
    Fp_set_ui(&res->x0, value);
    Fp_set_ui(&res->x1, value);
} 

void Fp2_copy_ne(Fp2 *res,Fp2 *A){
    Fp_copy_ne(&res->x0, &A->x0);
    Fp_copy_ne(&res->x1, &A->x1);
}

void Fp2_inv(Fp2 *res, Fp2 *A){
	Fp c_x0,c_x1,t0,t1;
    Fp_init(&c_x0);
    Fp_init(&c_x1);
    Fp_init(&t0);
    Fp_init(&t1);
    
    Fp_copy(&c_x0,&A->x0);
    Fp_copy_ne(&c_x1,&A->x1);
    
    Fp_mul(&t0,&c_x0,&A->x0);
    Fp_mul(&t1,&c_x1,&A->x1);
    Fp_sub(&t0,&t0,&t1);
    Fp_inv(&t0,&t0);
    Fp_mul(&res->x0,&c_x0,&t0);
    Fp_mul(&res->x1,&c_x1,&t0);
    
    Fp_clear(&c_x0);
    Fp_clear(&c_x1);
    Fp_clear(&t0);
    Fp_clear(&t1);
}

void Fp2_sqr(Fp2 *res, Fp2 *A){
    Fp tmp1,tmp2;
    Fp_init(&tmp1);
    Fp_init(&tmp2);
    
    Fp_add(&tmp1, &A->x0, &A->x1);
	Fp_sub(&tmp2, &A->x0, &A->x1);
	//x1
	Fp_mul(&res->x1, &A->x0, &A->x1);
	Fp_add(&res->x1, &res->x1, &res->x1);
	//x0
	Fp_mul(&res->x0, &tmp1, &tmp2);
    
    Fp_clear(&tmp1);
    Fp_clear(&tmp2);
}

void Fp2_mul(Fp2 *res, Fp2 *A, Fp2 *B){
    Fp temp1, temp2, temp3, temp4;
    Fp_init(&temp1);
    Fp_init(&temp2);
    Fp_init(&temp3);
    Fp_init(&temp4);
    Fp_mul(&temp1, &A->x0, &B->x0);//a*c
    Fp_mul(&temp2, &A->x1, &B->x1);//b*d
    Fp_add(&temp3, &A->x0, &A->x1);//a+b
    Fp_add(&temp4, &B->x0, &B->x1);//c+d
    //x0
    Fp_sub(&res->x0 ,&temp1, &temp2);//a*c-b*d*v
    //x1
    Fp_mul(&res->x1, &temp3, &temp4);//(a+b)(c+d)
    Fp_sub(&res->x1, &res->x1, &temp1);
    Fp_sub(&res->x1, &res->x1, &temp2);
    
    Fp_clear(&temp1);
    Fp_clear(&temp2);
    Fp_clear(&temp3);
    Fp_clear(&temp4);
}

void Fp2_mul_u(Fp2 *res, Fp2 *A, unsigned long int B){
	Fp_mul_u(&res->x0, &A->x0, B);
	Fp_mul_u(&res->x1, &A->x1, B);
}

void Fp2_mul_mpz(Fp2 *res, Fp2 *A, mpz_t B){
	Fp_mul_mpz(&res->x0, &A->x0, B);
    Fp_mul_mpz(&res->x1, &A->x1, B);
}

void Fp2_mul_basis(Fp2 *res, Fp2 *A){
	Fp temp1;
	Fp_init(&temp1);
	
	Fp_copy(&temp1, &A->x0);
    
    Fp_sub(&res->x0, &temp1, &A->x1);
    Fp_add(&res->x1, &temp1, &A->x1);
    
    Fp_clear(&temp1);
}

void Fp2_add(Fp2 *res, Fp2 *A, Fp2 *B){
	Fp_add(&res->x0, &A->x0, &B->x0);
	Fp_add(&res->x1, &A->x1, &B->x1);
}

void Fp2_sub(Fp2 *res, Fp2 *A, Fp2 *B){
	Fp_sub(&res->x0, &A->x0, &B->x0);
	Fp_sub(&res->x1, &A->x1, &B->x1);
}

void Fp2_exp(Fp2 *res, Fp2 *A, mpz_t sk){
	int i,length;
    length=(int)mpz_sizeinbase(sk,2);
    char binary[length];
    mpz_get_str(binary,2,sk);
    Fp2 tmp;
    Fp2_init(&tmp);
    Fp2_copy(&tmp,A);
    
    for(i=1; binary[i]!='\0'; i++){
        Fp2_sqr(&tmp,&tmp);
        if(binary[i]=='1'){
            Fp2_mul(&tmp,A,&tmp);
        }
    }
    
    Fp2_copy(res,&tmp);
    Fp2_clear(&tmp);
}

void Fp2_set_random(Fp2 *ANS){
    Fp_get_random(&ANS->x0);
    Fp_get_random(&ANS->x1);
}

int  Fp2_cmp_one(Fp2 *A){
    if(Fp_cmp_one(&A->x0)==0 && Fp_cmp_zero(&A->x1)==0){
        return 0;
    }
    return 1;
}

int  Fp2_legendre(Fp2 *A){
    Fp2 tmp;
    Fp2_init(&tmp);
    
    mpz_t exp;
    mpz_init(exp);
    mpz_pow_ui(exp,prime,2);
    mpz_sub_ui(exp,exp,1);
    mpz_tdiv_q_ui(exp,exp,2);
    Fp2_exp(&tmp,A,exp);
    
    if(Fp2_cmp_one(&tmp)==0){
        mpz_clear(exp);
        Fp2_clear(&tmp);
        return 1;
    }else{
        mpz_clear(exp);
        Fp2_clear(&tmp);
        return -1;
    }
}

void Fp2_sqrt(Fp2 *ANS,Fp2 *A){
    Fp2 x,y,t,k,n,tmp;
    Fp2_init(&x);
    Fp2_init(&y);
    Fp2_init(&t);
    Fp2_init(&k);
    Fp2_init(&n);
    Fp2_init(&tmp);
    unsigned long int e,m;
    mpz_t exp,q,z,result;
    mpz_init(exp);
    mpz_init(q);
    mpz_init(z);
    mpz_init(result);
    //gmp_randstate_t state;
	//gmp_randinit_default(state);
	//gmp_randseed_ui(state,(unsigned long)time(NULL));
    
    Fp2_set_random(&n);
    while(Fp2_legendre(&n)!=-1){
        Fp2_set_random(&n);
    }
    mpz_pow_ui(q,prime,2);
    mpz_sub_ui(q,q,1);
    mpz_mod_ui(result,q,2);
    e=0;
    while(mpz_cmp_ui(result,0)==0){
        mpz_tdiv_q_ui(q,q,2);
        mpz_mod_ui(result,q,2);
        e++;
    }
    Fp2_exp(&y,&n,q);
    mpz_set_ui(z,e);
    mpz_sub_ui(exp,q,1);
    mpz_tdiv_q_ui(exp,exp,2);
    Fp2_exp(&x,A,exp);
    Fp2_mul(&tmp,&x,&x);
    Fp2_mul(&k,&tmp,A);
    Fp2_mul(&x,&x,A);
    while(Fp2_cmp_one(&k)!=0){
        m=1;
        mpz_ui_pow_ui(exp,2,m);
        Fp2_exp(&tmp,&k,exp);
        while(Fp2_cmp_one(&tmp)!=0){
            m++;
            mpz_ui_pow_ui(exp,2,m);
            Fp2_exp(&tmp,&k,exp);
        }
        mpz_sub_ui(exp,z,m);
        mpz_sub_ui(exp,exp,1);
        mpz_ui_pow_ui(result,2,mpz_get_ui(exp));
        Fp2_exp(&t,&y,result);
        Fp2_mul(&y,&t,&t);
        mpz_set_ui(z,m);
        Fp2_mul(&x,&x,&t);
        Fp2_mul(&k,&k,&y);
    }
    Fp2_copy(ANS,&x);
    
    mpz_clear(exp);
    mpz_clear(q);
    mpz_clear(z);
    mpz_clear(result);
    Fp2_clear(&x);
    Fp2_clear(&y);
    Fp2_clear(&t);
    Fp2_clear(&k);
    Fp2_clear(&n);
    Fp2_clear(&tmp);
}
