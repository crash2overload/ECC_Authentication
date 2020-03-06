#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/miller.h"
#include "../include/curves.h"
/******************Line - Functions für Tate and ate ******/
#define prec 192

void Pseudo_8_sparse_mapping(EFp *P, EFp2 *Q, Fp *L){
    
    EFp2 Tmp_Q;
	EFp2_init(&Tmp_Q);
	EFp Tmp_P;
	EFp_init(&Tmp_P);
	Fp A,B,C,D,c;
	Fp_init(&A);
	Fp_init(&B);
	Fp_init(&C);
	Fp_init(&D);
	Fp_init(&c);
	
	EFp_copy(&Tmp_P,P);
	EFp2_copy(&Tmp_Q,Q);
	
	Fp_mul(&A,&Tmp_P.x,&Tmp_P.y);
	Fp_inv(&A, &A);
	Fp_mul(&B,&Tmp_P.x,&Tmp_P.x);
	Fp_mul(&B,&B,&A);
	Fp_mul(&C,&Tmp_P.y,&A);
	Fp_mul(&D,&B,&B);
	
	Fp2_mul_mpz(&Q->x,&Tmp_Q.x,D.x0);
	Fp_mul(&c,&B,&D);
	Fp2_mul_mpz(&Q->y,&Tmp_Q.y,c.x0);
	
	Fp_mul(&P->x,&D,&Tmp_P.x);
	Fp_copy(&P->y,&P->x);
	
	Fp_mul(L,&C,&Tmp_P.y);
	Fp_mul(L,L,L);
	Fp_mul(L,L,&C);
	
	
	EFp2_clear(&Tmp_Q);
	EFp_clear(&Tmp_P);
	Fp_clear(&A);
	Fp_clear(&B);
	Fp_clear(&C);
	Fp_clear(&D);
	Fp_clear(&c);
}

void Pseudo_8_sparse_mul(Fp12 *res, Fp12 *A, Fp12 *B){
    //A= f0 + f1γ^2 + f2γ^4 + f3γ　+ f4γ^3 + f5γ^5
	//B= 1                  +  aγ +  bγ^3
	// x0.x0  x0.x1  x0.x2  x1.x0   x1.x1   x1.x2
	Fp12 ans;
	Fp12_init(&ans);
	Fp2 tmp0,tmp1,tmp2,tmp3;
	Fp2_init(&tmp0);
	Fp2_init(&tmp1);
	Fp2_init(&tmp2);
	Fp2_init(&tmp3);
	
	Fp2_mul(&tmp0, &A->x0.x0, &B->x1.x0);		//tmp0=b3*f0
	Fp2_mul(&tmp1, &A->x0.x1, &B->x1.x1);		//tmp1=b4*f1
	Fp2_add(&tmp2, &A->x0.x0, &A->x0.x1);		//tmp2=f0+f1
	Fp2_add(&tmp3, &B->x1.x0, &B->x1.x1);		//tmp3=b3+b4
	Fp2_mul(&tmp2, &tmp2, &tmp3);			//tmp2=tmp2*tmp3
	Fp2_sub(&tmp2, &tmp2, &tmp0);			//tmp2=tmp2-tmp0
	Fp2_sub(&tmp2, &tmp2, &tmp1);			//tmp2=tmp2-tmp1
	Fp2_add(&ans.x1.x1, &tmp2, &A->x1.x1);	//ans[γ^3]=tmp2+f4
	Fp2_add(&tmp0, &tmp0, &A->x1.x0);		//tmp0=tmp0+f3
	Fp2_mul(&tmp2, &A->x0.x2, &B->x1.x1);		//tmp2=b4*f2
	Fp2_mul_basis(&tmp2, &tmp2);			//tmp2=tmp2*(α+1)
	Fp2_add(&ans.x1.x0, &tmp0, &tmp2);		//ans[γ]=tmp0+tmp2
	Fp2_add(&tmp0, &tmp1, &A->x1.x2);		//tmp0=tmp1+f5
	Fp2_mul(&tmp1, &A->x0.x2, &B->x1.x0);		//tmp1=b3*f2
	Fp2_add(&ans.x1.x2, &tmp0, &tmp1);		//ans[γ^5]=tmp0+tmp1
	Fp2_mul(&tmp0, &A->x1.x0, &B->x1.x0);		//tmp0=b3*f3
	Fp2_mul(&tmp1, &A->x1.x1, &B->x1.x1);		//tmp1=b4*f4
	Fp2_add(&tmp2, &A->x1.x0, &A->x1.x1);		//tmp2=f3+f4
	Fp2_mul(&tmp2, &tmp2, &tmp3);			//tmp2=tmp2*tmp3
	Fp2_sub(&tmp2, &tmp2, &tmp0);			//tmp2=tmp2-tmp0
	Fp2_sub(&tmp2, &tmp2, &tmp1);			//tmp2=tmp2-tmp1
	Fp2_add(&ans.x0.x2, &tmp2, &A->x0.x2);	//ans[γ^4]=tmp2+f4
	Fp2_add(&tmp0, &tmp0, &A->x0.x1);		//tmp0=tmp0+f1
	Fp2_mul(&tmp2, &A->x1.x2, &B->x1.x1);		//tmp2=b4*f5
	Fp2_mul_basis(&tmp2, &tmp2);			//tmp2=tmp2*(α+1)
	Fp2_add(&ans.x0.x1, &tmp0,&tmp2);		//ans[γ^2]=tmp0+tmp2
	Fp2_mul(&tmp0, &A->x1.x2, &B->x1.x0);		//tmp0=b3*f5
	Fp2_add(&tmp0, &tmp0, &tmp1);			//tmp0=tmp0+tmp1
	Fp2_mul_basis(&tmp0, &tmp0);			//tmp0=tmp0*(α+1)
	Fp2_add(&ans.x0.x0, &tmp0, &A->x0.x0);	//ans[1]=tmp0+f0
	
	Fp12_copy(res, &ans);
	
	Fp12_clear(&ans);
	Fp2_clear(&tmp0);
	Fp2_clear(&tmp1);
	Fp2_clear(&tmp2);
	Fp2_clear(&tmp3);
}


void ff_ltt(Fp12 *f, EFp2 *T, EFp2 *Q, EFp *P){
    
	Fp2 A,B,C,D,E, F, tmp1, tmp2;
	Fp2_init(&A);
	Fp2_init(&B);
	Fp2_init(&C);
	Fp2_init(&D);
	Fp2_init(&E);
	Fp2_init(&F);
	Fp2_init(&tmp1);
	Fp2_init(&tmp2);
	
	
	Fp2_sqr(&A, &Q->x);
	Fp2_sqr(&B, &Q->y);
	Fp2_sqr(&C, &B);
	Fp2_add(&D, &Q->x, &B);
	Fp2_sqr(&D, &D);
	Fp2_sub(&D, &D, &A);
	Fp2_sub(&D, &D, &C);
	Fp2_mul_u(&D, &D, 2);
	Fp2_mul_u(&E, &A, 3);
	
	Fp2_add(&tmp1, &E, &Q->x);
	Fp2_sqr(&F, &E);
	Fp2_sub(&T->x, &F, &D);
	Fp2_sub(&T->x, &T->x, &D); //X3
	
	Fp2_mul(&T->z, &Q->y, &Q->z);
	Fp2_mul_u(&T->z, &T->z, 2); //Z3
	
	Fp2_sub(&T->y, &D, &T->x);
	Fp2_mul(&T->y, &T->y, &E);
	Fp2_mul_u(&tmp2, &C, 8);
	Fp2_sub(&T->y, &T->y, &tmp2); //Y3
	
	
	EFp2_clear(&Tmp_T);
	Fp2_clear(&A);
	Fp2_clear(&B);
	Fp2_clear(&C);
	Fp2_clear(&D);
	Fp2_clear(&E);
	Fp2_clear(&F);
	Fp2_clear(&tmp1);
	Fp2_clear(&tmp2);
}


void f_ltq(Fp12 *f, EFp2 *T, EFp2 *Q, EFp *P){
    
    EFp2 Tmp_T;
	EFp2_init(&Tmp_T);
	Fp12 ltq;
	Fp12_init(&ltq);
	Fp2 A,B,C,D,E;
	Fp2_init(&A);
	Fp2_init(&B);
	Fp2_init(&C);
	Fp2_init(&D);
	Fp2_init(&E);
	EFp2_copy(&Tmp_T,T);
		
	//ltq
	Fp2_sqr(&A, &Q->x);
	
	//set ltq
	//Fp_set_ui(&ltq.x0.x0.x0,1);
	//Fp2_copy_ne(&ltq.x1.x0,&C);
	//Fp2_mul_mpz(&ltq.x1.x1,&E,L->x0);
	
	//Pseudo_8_sparse_mul(f,f,&ltq);
	
	EFp2_clear(&Tmp_T);
	Fp12_clear(&ltq);
	Fp2_clear(&A);
	Fp2_clear(&B);
	Fp2_clear(&C);
	Fp2_clear(&D);
}

void Miller_ate(Fp12 *res, EFp2 *Q, EFp *P){
    
    EFp2 r;
    EFp2_init(&r);
    EFp2_copy(&r, Q);
    
    Fp2 r2;
    Fp2_init(&r2);
    Fp2_sqr(&r2, &Q->y);
    //EFp2_init(&T);
    //EFp2 mapped_Q;
    //EFp2_init(&mapped_Q);
    //EFp mapped_P;
    //EFp_init(&mapped_P);
    Fp12 f;
    Fp12_init(&f);
    
    //Fp L;
    //Fp_init(&L);

	mpz_t loop, trace;
	mpz_init(loop);
	mpz_init(trace);
	mpz_set_str(trace, "79228149111146082285597799927", 10);
	mpz_sub_ui(loop,trace,1);
	int i,length;
	length=(int)mpz_sizeinbase(loop,2);
	char binary[length];
	mpz_get_str(binary,2,loop);
	
    //EFp_copy(&mapped_P,P);
    //EFp2_copy(&mapped_Q,Q);
    //Pseudo_8_sparse_mapping(&mapped_P,&mapped_Q,&L);
    //EFp2_copy(&T,&mapped_Q);     //set T
    Fp_set_ui(&f.x0.x0.x0,1);   //set f
    
    //__u32 hilfs = 0;
    //hilfs = 1 << 31;
    //miller
    for(i=1; binary[i]!='\0'; i++){
		ff_ltt(&f, &r, &r, P);
		Fp12_sqr(res, res);
		Fp12_mul(res, res, &f);
		
		if(binary[i]=='1'){
			//f_ltq(&f, &r, &r, P, Q, &r2);
			Fp12_mul(res, res, &f);
		}
	}
    
    ////miller
	//for	(j = 0; j < 3; j++) {
		//for(i = 0; i < 32; i++){
			//ff_ltt(&f,&T,&mapped_P,&L);
			//if(hilfs & bn192.trace[j]){
				//f_ltq(&f,&T,&mapped_Q,&mapped_P,&L);
		   //}
		   //bn192.trace[j] = bn192.trace[j] << 1;
		//}
		//printf("Erster durchgang \n");
	//}
      
    Fp12_copy(res, &f);
    
    Fp12_clear(&f);
    EFp2_clear(&r);
    
    Fp2_clear(&r2);
   
}

void Fp12_frobenius_map_p2(Fp12 *ANS,Fp12 *A){
    //x0
    Fp2_copy(&ANS->x0.x0,&A->x0.x0);
    Fp2_mul_mpz(&ANS->x0.x1,&A->x0.x1,frobenius_constant[0][1].x0.x0);
    Fp2_mul_mpz(&ANS->x0.x2,&A->x0.x2,frobenius_constant[0][2].x0.x0);
    //x1
    Fp2_mul_mpz(&ANS->x1.x0,&A->x1.x0,frobenius_constant[0][3].x0.x0);
    Fp2_copy_ne(&ANS->x1.x1,&A->x1.x1);
    Fp2_mul_mpz(&ANS->x1.x2,&A->x1.x2,frobenius_constant[0][5].x0.x0);
}

void Fp12_frobenius_map_p6(Fp12 *res,Fp12 *A){
    //x0
	Fp6_copy(&res->x0, &A->x0);
	//x1
	Fp6_set_ne(&res->x1, &A->x1);
}

void Final_exp(Fp12 *res, Fp12 *A){
    Fp12 Tmp,Buf1,Buf2;
	Fp12_init(&Tmp);
	Fp12_copy(&Tmp,A);
	Fp12_init(&Buf1);
	Fp12_init(&Buf2);
	mpz_t exp,buf, /*prime,*/ order;
	mpz_init(exp);
	mpz_init(buf);
	//mpz_init(prime);
	mpz_init(order);
	//mpz_set_str(prime, "6277099611578052373965416350240300935678131109448722797563", 10);
	mpz_set_str(order, "6277099611578052373965416350161072786566985027163124997637", 10);
	
	printf("\n leichter Teil\n");
	Fp12_frobenius_map_p6(&Buf1,&Tmp);
	Fp12_inv(&Buf2,&Tmp);
	
	Fp12_mul(&Tmp,&Buf1,&Buf2);
	
	Fp12_frobenius_map_p2(&Buf1,&Tmp);
	
	Fp12_mul(&Tmp,&Buf1,&Tmp);

	printf("\n leichter Teil ende\n");
	
	mpz_pow_ui(exp,prime,4);
	mpz_pow_ui(buf,prime,2);
	mpz_sub(exp,exp,buf);
	mpz_add_ui(exp,exp,1);
	mpz_tdiv_q(exp,exp,order);
	Fp12_print(&Tmp);
	gmp_printf ("exp %Zd\n", exp);
	Fp12_exp(res,&Tmp,exp);
	
	mpz_clear(exp);
	mpz_clear(buf);
	mpz_clear(prime);
	Fp12_clear(&Tmp);
	Fp12_clear(&Buf1);
	Fp12_clear(&Buf2);
}


