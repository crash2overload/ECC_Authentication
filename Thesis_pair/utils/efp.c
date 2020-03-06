#include <stdio.h>
#include <string.h>

#include "../include/efp.h"
#include "../include/curves.h"

#define prec 192


void EFp_init(EFp *P){
	Fp_init(&P->x);
	Fp_init(&P->y);
	Fp_init(&P->z);	
}

void EFp_clear(EFp *P){
	Fp_clear(&P->x);
	Fp_clear(&P->y);
	Fp_clear(&P->z);	
}

void EFp_print(EFp *P){
	Fp_print(&P->x);
	Fp_print(&P->y);
	Fp_print(&P->z);
}

void EFp_copy(EFp *dest, EFp *src){
	Fp_copy(&dest->x, &src->x);
	Fp_copy(&dest->y, &src->y);
	Fp_copy(&dest->z, &src->z);
}

void EFp_get_generator(EFp *P){
	memcpy(P->x.x0, &bn192.G_x[0], prec/8);
	memcpy(P->y.x0, &bn192.G_y[0], prec/8);
}

void EFp_to_affine(EFp *P){
	Fp tmp, tmp2;
	Fp_init(&tmp);
	Fp_init(&tmp2);
	Fp_mul(&tmp, &P->z, &P->z);
	Fp_inv(&tmp2, &tmp);
	Fp_mul(&tmp, &tmp, &P->z);
	Fp_mul(&P->x, &P->x, &tmp2);
	Fp_mul(&P->y, &P->y, &tmp);
	mpz_set_str(P->z.x0, "1", 10);
	Fp_clear(&tmp);
	Fp_clear(&tmp2);
}

void EFp_add(EFp *res, EFp *P, EFp *Q){
	//Z1Z1 = Z12
	//Z2Z2 = Z22
	//U1 = X1*Z2Z2
	//U2 = X2*Z1Z1
	//S1 = Y1*Z2*Z2Z2
	//S2 = Y2*Z1*Z1Z1
	//H = U2-U1
	//I = (2*H)2
	//J = H*I
	//r = 2*(S2-S1)
	//V = U1*I
	//X3 = r2-J-2*V
	//Y3 = r*(V-X3)-2*S1*J
	//Z3 = ((Z1+Z2)2-Z1Z1-Z2Z2)*H
	Fp zz1, zz2, u1, u2, s1, s2, h, i, j, r, rr, V;
	Fp_init(&zz1);
	Fp_init(&zz2);
	Fp_init(&u1);
	Fp_init(&u2);
	Fp_init(&s1);
	Fp_init(&s2);
	Fp_init(&h);
	Fp_init(&i);
	Fp_init(&j);
	Fp_init(&r);
	Fp_init(&rr);
	Fp_init(&V);
	
	Fp_mul(&zz1, &P->z, &P->z);	
	Fp_mul(&zz2, &Q->z, &Q->z);
	Fp_mul(&u1, &zz2, &P->x);
	Fp_mul(&u2, &zz1, &Q->x);
	Fp_mul(&s1, &zz2, &Q->z);
	Fp_mul(&s1, &s1, &P->y);
	Fp_mul(&s2, &zz1, &P->z);
	Fp_mul(&s2, &s2, &Q->y);
	Fp_sub(&h, &u2, &u1);
	Fp_mul_u(&i, &h, 2);
	Fp_mul(&i, &i, &i);
	Fp_mul(&j, &h, &i);
	Fp_sub(&r, &s2, &s1);
	Fp_mul_u(&r, &r, 2);
	Fp_mul(&V, &u1, &i);
	Fp_mul(&res->x, &r, &r);
	Fp_sub(&res->x, &res->x, &j);
	Fp_mul_u(&rr, &V, 2);
	Fp_sub(&res->x, &res->x, &rr);
	Fp_sub(&res->y, &V, &res->x);
	Fp_mul(&res->y, &res->y, &r);
	Fp_mul(&s1, &s1, &j);
	Fp_mul_u(&s1, &s1, 2);
	Fp_sub(&res->y, &res->y, &s1);
	Fp_add(&res->z, &P->z, &Q->z);
	Fp_mul(&res->z, &res->z, &res->z);
	Fp_sub(&res->z, &res->z, &zz1);
	Fp_sub(&res->z, &res->z, &zz2);
	Fp_mul(&res->z, &res->z, &h);
	
	
	Fp_clear(&zz1);
	Fp_clear(&zz2);
	Fp_clear(&u1);
	Fp_clear(&u2);
	Fp_clear(&s1);
	Fp_clear(&s2);
	Fp_clear(&h);
	Fp_clear(&i);
	Fp_clear(&j);
	Fp_clear(&r);
	Fp_clear(&rr);
	Fp_clear(&V);
	//EFp_to_proj(P);
	//EFp_to_proj(Q);
	//EFp_add_core(res, P, Q);
	//EFp_to_affine(res);
	//EFp_to_affine(P);
	//EFp_to_affine(Q);
}

void EFp_dbb(EFp *res, EFp *P){
	//A = X1^2
	//B = Y1^2
	//C = B2
	//D = 2*((X1+B)2-A-C)
	//E = 3*A
	//F = E2
	//X3 = F-2*D
	//Y3 = E*(D-X3)-8*C
	//Z3 = 2*Y1*Z1
	Fp a, b, c, d, e, f;
	EFp tmp;
	EFp_init(&tmp);
	EFp_copy(&tmp, P);
	Fp_init(&a);
	Fp_init(&b);
	Fp_init(&c);
	Fp_init(&d);
	Fp_init(&e);
	Fp_init(&f);
	
	Fp_mul(&a, &tmp.x, &tmp.x);
	Fp_mul(&b, &tmp.y, &tmp.y);
	Fp_mul(&c, &b, &b);
	Fp_add(&d, &tmp.x, &b);
	Fp_mul(&d, &d, &d);
	Fp_sub(&d, &d, &a);
	Fp_sub(&d, &d, &c);
	Fp_mul_u(&d, &d, 2);
	Fp_mul_u(&e, &a, 3);
	Fp_mul(&f, &e, &e);
	Fp_mul_u(&res->x, &d, 2);
	Fp_sub(&res->x, &f, &res->x);
	Fp_sub(&res->y, &d, &res->x);
	Fp_mul(&res->y, &e, &res->y);
	Fp_mul_u(&c, &c, 8);
	Fp_sub(&res->y, &res->y, &c);
	Fp_mul(&res->z, &tmp.y, &tmp.z);
	Fp_mul_u(&res->z, &res->z, 2);
	
	Fp_clear(&a);
	Fp_clear(&b);
	Fp_clear(&c);
	Fp_clear(&d);
	Fp_clear(&e);
	Fp_clear(&f);
}

void EFp_mul(EFp *res, mpz_t sk, EFp *P){
	EFp Tmp_P, Next_P;
    EFp_init(&Tmp_P);
    EFp_copy(&Tmp_P, P);
    EFp_init(&Next_P);
    int i,length;
    length=(int)mpz_sizeinbase(sk, 2);
    char binary[length];
    mpz_get_str(binary, 2, sk);
    
    EFp_copy(&Next_P, &Tmp_P);
    for(i=1; binary[i]!='\0'; i++){
        EFp_dbb(&Next_P, &Next_P);
        if(binary[i]=='1'){
            EFp_add(&Next_P, &Next_P, &Tmp_P);
        }
    }
    
    EFp_copy(res, &Next_P);
    
    EFp_clear(&Next_P);
    EFp_clear(&Tmp_P);
	
	//EFp_to_proj(P);
	//EFp_mul_core(sk->x0, P);
	//EFp_to_affine(P);
}

void EFp2_init(EFp2 *P){
	Fp2_init(&P->x);
	Fp2_init(&P->y);
	Fp2_init(&P->z);
}
void EFp2_clear(EFp2 *P){
	Fp2_clear(&P->x);
	Fp2_clear(&P->y);
	Fp2_clear(&P->z);
}

void EFp2_copy(EFp2 *dest, EFp2 *src){
	Fp2_copy(&dest->x, &src->x);
	Fp2_copy(&dest->y, &src->y);
	Fp2_copy(&dest->z, &src->z);
}

void EFp2_set_ui(EFp2 *ANS,unsigned long int UI){
    Fp2_set_ui(&ANS->x,UI);
    Fp2_set_ui(&ANS->y,UI);
    //ANS->infinity=0;
}

/*void EFp2_set_mpz(EFp2 *ANS,mpz_t A){
    Fp2_set_mpz(&ANS->x,A);
    Fp2_set_mpz(&ANS->y,A);
    //ANS->infinity=0;
}*/

void EFp2_set_neg(EFp2 *ANS,EFp2 *A){
    Fp2_copy(&ANS->x,&A->x);
    Fp2_copy_ne(&ANS->y,&A->y);
    //ANS->infinity=A->infinity;
}

void EFp2_printf(EFp2 *P,char *str){
    printf("%s",str);
    //if(P->infinity==0){
        printf("(");
        Fp2_print(&P->x);
        printf(",");
        Fp2_print(&P->y);
        printf(")");
    //}else{
    //    printf("0");
    //}
}

void EFp2_rational_point(EFp2 *P){
    Fp2 tmp1,tmp2;
    Fp curve_b;
    Fp_init(&curve_b);
    Fp_set_ui(&curve_b, 28);
    Fp2_init(&tmp1);
    Fp2_init(&tmp2);
	//gmp_randinit_default (state);
	//gmp_randseed_ui(state,(unsigned long)time(NULL));
    
    while(1){
        Fp2_set_random(&P->x);
        Fp2_sqr(&tmp1,&P->x);
        Fp2_mul(&tmp2,&tmp1,&P->x);
        Fp_add(&tmp2.x0,&tmp2.x0,&curve_b);
        if(Fp2_legendre(&tmp2)==1){
            Fp2_sqrt(&P->y,&tmp2);
            break;
        }
    }
    
    Fp2_clear(&tmp1);
    Fp2_clear(&tmp2);
}
/*
void EFp2_ECD(EFp2 *ANS,EFp2 *P){
    if(Fp2_cmp_zero(&P->y)==0){
        ANS->infinity=1;
        return;
    }
    
    EFp2 Tmp_P;
    EFp2_init(&Tmp_P);
    EFp2_set(&Tmp_P,P);
    Fp2 tmp1,tmp2,lambda;
    Fp2_init(&tmp1);
    Fp2_init(&tmp2);
    Fp2_init(&lambda);
    
    Fp2_add(&tmp1,&Tmp_P.y,&Tmp_P.y);
    
    Fp2_inv(&tmp1,&tmp1);
    Fp2_sqr(&tmp2,&Tmp_P.x);
    Fp2_add(&lambda,&tmp2,&tmp2);
    Fp2_add(&tmp2,&tmp2,&lambda);
    Fp2_mul(&lambda,&tmp1,&tmp2);
    
    Fp2_sqr(&tmp1,&lambda);
    Fp2_add(&tmp2,&Tmp_P.x,&Tmp_P.x);
    Fp2_sub(&ANS->x,&tmp1,&tmp2);
    
    Fp2_sub(&tmp1,&Tmp_P.x,&ANS->x);
    Fp2_mul(&tmp2,&lambda,&tmp1);
    Fp2_sub(&ANS->y,&tmp2,&Tmp_P.y);
    
    Fp2_clear(&tmp1);
    Fp2_clear(&tmp2);
    Fp2_clear(&lambda);
    EFp2_clear(&Tmp_P);
}

void EFp2_ECA(EFp2 *ANS,EFp2 *P1,EFp2 *P2){
    if(P1->infinity==1){
        EFp2_set(ANS,P2);
        return;
    }else if(P2->infinity==1){
        EFp2_set(ANS,P1);
        return;
    }else if(Fp2_cmp(&P1->x,&P2->x)==0){
        if(Fp2_cmp(&P1->y,&P2->y)!=0){
            ANS->infinity=1;
            return;
        }else{
            EFp2_ECD(ANS,P1);
            return;
        }
    }
    
    EFp2 Tmp_P1,Tmp_P2;
    EFp2_init(&Tmp_P1);
    EFp2_set(&Tmp_P1,P1);
    EFp2_init(&Tmp_P2);
    EFp2_set(&Tmp_P2,P2);
    Fp2 tmp1,tmp2,lambda;
    Fp2_init(&tmp1);
    Fp2_init(&tmp2);
    Fp2_init(&lambda);
    
    Fp2_sub(&tmp1,&Tmp_P2.x,&Tmp_P1.x);
    Fp2_inv(&tmp1,&tmp1);
    Fp2_sub(&tmp2,&Tmp_P2.y,&Tmp_P1.y);
    Fp2_mul(&lambda,&tmp1,&tmp2);
    Fp2_sqr(&tmp1,&lambda);
    Fp2_sub(&tmp2,&tmp1,&Tmp_P1.x);
    Fp2_sub(&ANS->x,&tmp2,&Tmp_P2.x);
    Fp2_sub(&tmp1,&Tmp_P1.x,&ANS->x);
    Fp2_mul(&tmp2,&lambda,&tmp1);
    Fp2_sub(&ANS->y,&tmp2,&Tmp_P1.y);
        
    //clear 
    Fp2_clear(&tmp1);
    Fp2_clear(&tmp2);
    Fp2_clear(&lambda);
    EFp2_clear(&Tmp_P1);
    EFp2_clear(&Tmp_P2);
}

void EFp2_SCM(EFp2 *ANS,EFp2 *P,mpz_t scalar){
    if(mpz_cmp_ui(scalar,0)==0){
        ANS->infinity=1;
        return;
    }else if(mpz_cmp_ui(scalar,1)==0){
        EFp2_set(ANS,P);
        return;
    }
    
    EFp2 Tmp_P,Next_P;
    EFp2_init(&Tmp_P);
    EFp2_set(&Tmp_P,P);
    EFp2_init(&Next_P);
    int i,length;
    length=(int)mpz_sizeinbase(scalar,2);
    char binary[length];
    mpz_get_str(binary,2,scalar);
    
    EFp2_set(&Next_P,&Tmp_P);
    for(i=1; binary[i]!='\0'; i++){
        EFp2_ECD(&Next_P,&Next_P);
        if(binary[i]=='1'){
            EFp2_ECA(&Next_P,&Next_P,&Tmp_P);
        }
    }
    EFp2_set(ANS,&Next_P);
    
    EFp2_clear(&Next_P);
    EFp2_clear(&Tmp_P);
}*/
//void EFp2_print(EFp2 *P);
