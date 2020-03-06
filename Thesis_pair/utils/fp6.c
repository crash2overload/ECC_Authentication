#include "../include/fp6.h"

void Fp6_init(Fp6 *A){
	Fp2_init(&A->x0);
	Fp2_init(&A->x1);
	Fp2_init(&A->x2);
}
void Fp6_clear(Fp6 *A){
	Fp2_clear(&A->x0);
	Fp2_clear(&A->x1);
	Fp2_clear(&A->x2);
}

void Fp6_print(Fp6 *A){
	Fp2_print(&A->x0);
	Fp2_print(&A->x1);
	Fp2_print(&A->x2);
}


void Fp6_inv(Fp6 *res, Fp6 *A){
	Fp6 barA;
    Fp6_init(&barA);
    Fp2 s,t0,t1,t2,t3;
    Fp2_init(&s);
    Fp2_init(&t0);
    Fp2_init(&t1);
    Fp2_init(&t2);
    Fp2_init(&t3);
    
    Fp2_sqr(&t0, &A->x0); //t0=a0^2
    Fp2_sqr(&t1, &A->x1); //t1=a1^2
    Fp2_sqr(&t2, &A->x2); //t2=a2^2
    
    Fp2_mul(&t3, &A->x1, &A->x2);
    Fp2_mul_basis(&t3, &t3);
    Fp2_sub(&barA.x0, &t0, &t3);
    
    Fp2_mul(&t3, &A->x0, &A->x1);
    Fp2_mul_basis(&barA.x1, &t2);
    Fp2_sub(&barA.x1, &barA.x1, &t3);
    
    Fp2_mul(&t3, &A->x0, &A->x2);
    Fp2_sub(&barA.x2, &t1, &t3);
    
    Fp2_mul(&t0, &t0, &A->x0);
    Fp2_mul(&t2, &t2, &A->x2);
    Fp2_mul_basis(&t2, &t2);
    
    Fp2_add(&s, &t3, &t3);
    Fp2_add(&s, &s, &t3);
    Fp2_sub(&s, &t1, &s);
    Fp2_mul(&s, &s, &A->x1);
    Fp2_add(&s, &s, &t2);
    Fp2_mul_basis(&s, &s);
    Fp2_add(&s, &s, &t0);
    
    Fp2_inv(&s, &s);
    
    Fp2_mul(&res->x0, &barA.x0, &s);
    Fp2_mul(&res->x1, &barA.x1, &s);
    Fp2_mul(&res->x2, &barA.x2, &s);
    
    Fp6_clear(&barA);
    Fp2_clear(&s);
    Fp2_clear(&t0);
    Fp2_clear(&t1);
    Fp2_clear(&t2);
    Fp2_clear(&t3);
}

void Fp6_set_ne(Fp6 *res, Fp6 *A){
	Fp2_copy_ne(&res->x0, &A->x0);
    Fp2_copy_ne(&res->x1, &A->x1);
    Fp2_copy_ne(&res->x2, &A->x2);
}

void Fp6_copy(Fp6 *dest, Fp6 *src){
	Fp2_copy(&dest->x0, &src->x0);
	Fp2_copy(&dest->x1, &src->x1);
	Fp2_copy(&dest->x2, &src->x2);
}

void Fp6_mul(Fp6 *res, Fp6 *A, Fp6 *B){
	Fp2 temp1, temp2, temp3, temp4, temp5, temp6, temp7;
    Fp2_init(&temp1);
    Fp2_init(&temp2);
    Fp2_init(&temp3);
    Fp2_init(&temp4);
    Fp2_init(&temp5);
    Fp2_init(&temp6);
    Fp2_init(&temp7);
	
	Fp2_mul(&temp1, &A->x0, &B->x0);//x0*y0
    Fp2_mul(&temp2, &A->x1, &B->x1);//x1*y1
    Fp2_mul(&temp3, &A->x2, &B->x2);//x2*y2
    
    Fp2_add(&temp5, &A->x0, &A->x1);//x0+x1
    Fp2_add(&temp4, &B->x0, &B->x1);//y0+y1
    Fp2_mul(&temp5, &temp5, &temp4);//(x0+x1)(y0+y1)
    
    Fp2_add(&temp6, &A->x1, &A->x2);//x1+x2
    Fp2_add(&temp4, &B->x1, &B->x2);//y1+y2
    Fp2_mul(&temp6, &temp6, &temp4);//(x1+x2)(y1+y2)
    
    Fp2_add(&temp7, &B->x0, &B->x2);//y2+y0
    Fp2_add(&temp4, &A->x0, &A->x2);//x2+x0
    Fp2_mul(&temp7, &temp7, &temp4);//(x2+x0)(y2+y0)
    //x0
    Fp2_sub(&temp6, &temp6, &temp2);
    Fp2_sub(&temp6, &temp6, &temp3);//(x1+x2)(y1+y2)-x1y1-x2y2
    Fp2_mul_basis(&temp4, &temp6);
    Fp2_add(&res->x0, &temp1, &temp4);
    //x1
    Fp2_sub(&temp5, &temp5, &temp1);
    Fp2_sub(&temp5, &temp5, &temp2);
    Fp2_mul_basis(&temp4, &temp3);
    Fp2_add(&res->x1, &temp4, &temp5);
    //x2
    Fp2_sub(&temp7, &temp7, &temp1);
    Fp2_sub(&temp7, &temp7, &temp3);
    Fp2_add(&res->x2, &temp2, &temp7);
    
    Fp2_clear(&temp1);
    Fp2_clear(&temp2);
    Fp2_clear(&temp3);
    Fp2_clear(&temp4);
    Fp2_clear(&temp5);
    Fp2_clear(&temp6);
    Fp2_clear(&temp7);
}

void Fp6_mul_basis(Fp6 *res, Fp6 *A){
    Fp2 temp1, temp2, temp3;
    Fp2_init(&temp1);
    Fp2_init(&temp2);
    Fp2_init(&temp3);
    
    Fp2_copy(&temp1, &A->x0);
    Fp2_copy(&temp2, &A->x1);
    Fp2_copy(&temp3, &A->x2);
    
    Fp_sub(&res->x0.x0, &temp3.x0, &temp3.x1);
    Fp_add(&res->x0.x1, &temp3.x0, &temp3.x1);
    Fp_copy(&res->x1.x0, &temp1.x0);
    Fp_copy(&res->x1.x1, &temp1.x1);
    Fp_copy(&res->x2.x0, &temp2.x0);
    Fp_copy(&res->x2.x1, &temp2.x1);
    
    Fp2_clear(&temp1);
    Fp2_clear(&temp2);
    Fp2_clear(&temp3);
}

void Fp6_add(Fp6 *res, Fp6 *A, Fp6 *B){
	Fp2_add(&res->x0, &A->x0, &B->x0);
	Fp2_add(&res->x1, &A->x1, &B->x1);
	Fp2_add(&res->x2, &A->x2, &B->x2);
}

void Fp6_sub(Fp6 *res, Fp6 *A, Fp6 *B){
	Fp2_sub(&res->x0, &A->x0, &B->x0);
	Fp2_sub(&res->x1, &A->x1, &B->x1);
	Fp2_sub(&res->x2, &A->x2, &B->x2);
}

void Fp6_exp(Fp6 *res, Fp6 *A, Fp6 *sk){
	
}

