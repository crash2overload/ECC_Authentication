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
	
	Fp2_sqr(&tmp2, &Q->z);
	Fp2_mul(&D, &E, &tmp2);
	Fp2_mul_u(&D, &D, 2);
	Fp2_copy_ne(&D, &D);
	Fp_mul(&D.x0, &D.x0, &P->x);
	Fp_mul(&D.x1, &D.x1, &P->x);
	
	Fp2_sqr(&tmp1, &tmp1);
	Fp2_sub(&tmp1, &tmp1, &A);
	Fp2_sub(&tmp1, &tmp1, &F);
	Fp2_mul_u(&tmp2, &B, 4);
	Fp2_sub(&tmp1, &tmp1, &tmp2);
	
	Fp2_sqr(&tmp2, &Q->z);
	Fp2_mul(&A, &T->z, &tmp2);
	Fp2_mul_u(&A, &A, 2);
	Fp_mul(&A.x0, &A.x0, &P->y);
	Fp_mul(&A.x1, &A.x1, &P->y);
	
	Fp2_set_ui(&B, 0);
	
	Fp6 part1, part2;
	Fp6_init(&part1);
	Fp6_init(&part2);
	
	Fp2_copy(&part1.x0, &B);
	Fp2_copy(&part1.x1, &tmp1);
	Fp2_copy(&part1.x2, &D);
	
	Fp2_copy(&part2.x0, &B);
	Fp2_copy(&part2.x1, &B);
	Fp2_copy(&part2.x2, &A);
	
	Fp6_copy(&f->x0, &part1);
	Fp6_copy(&f->x1, &part2);
	
	
	//EFp2_clear(&Tmp_T);
	Fp2_clear(&A);
	Fp2_clear(&B);
	Fp2_clear(&C);
	Fp2_clear(&D);
	Fp2_clear(&E);
	Fp2_clear(&F);
	Fp2_clear(&tmp1);
	Fp2_clear(&tmp2);
}


void f_ltq(Fp12 *f, EFp2 *rop2, EFp2 *T, EFp2 *Q, EFp *P, Fp2 *r2){
    
    Fp2 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11; // Temporary variables needed for intermediary results
    Fp2_init(&tmp0);
    Fp2_init(&tmp1);
    Fp2_init(&tmp2);
    Fp2_init(&tmp3);
    Fp2_init(&tmp4);
    Fp2_init(&tmp5);
    Fp2_init(&tmp6);
    Fp2_init(&tmp7);
    Fp2_init(&tmp8);
    Fp2_init(&tmp9);
    Fp2_init(&tmp10);
    Fp2_init(&tmp11);
    Fp6 tfp6e1, tfp6e2;
    Fp6_init(&tfp6e1);
    Fp6_init(&tfp6e2);


	Fp2_sqr(&tmp11, &T->z);
    Fp2_mul(&tmp0, &Q->x, &tmp11); /* tmp0 = B = x2 * T1  = x2z1^2*/

    Fp2_add(&tmp1, &Q->y, &T->z);
    Fp2_sqr(&tmp1, &tmp1);
    Fp2_sub(&tmp1, &tmp1, r2);
    Fp2_sub(&tmp1, &tmp1, &tmp11);
    Fp2_mul(&tmp1, &tmp1, &tmp11); /* tmp1 = D = ((y2 + Z1)^2 - R2 - T1)T1  = 2y2z1^3 */

    Fp2_sub(&tmp2, &tmp0, &T->x); /* tmp2 = H = B - X1  = x2z1^2 - x1*/

    Fp2_sqr(&tmp3, &tmp2); /* tmp3 = I = H^2  = (x2z1^2 - x1)^2*/

    Fp2_mul_u(&tmp4, &tmp3, 4);  /* tmp4 = E = 4I = 4(x2z1^2 - x1)^2*/

    Fp2_mul(&tmp5, &tmp2, &tmp4); /* tmp5 = J = HE =  4(x2z1^2 - x1)(x2z1^2 - x1)^2*/

    Fp2_sub(&tmp6, &tmp1, &T->y); 
    Fp2_sub(&tmp6, &tmp6, &T->y); /* tmp6 = r = 2(D - 2Y1) = (2y2z1^3 - 2y1)*/
    
    Fp2_mul(&tmp9, &tmp6, &Q->x); /* Needed later: tmp9 = x2(2y2z1^3 - 2y1)*/

    Fp2_mul(&tmp7, &T->x, &tmp4); /* tmp7 = V = X1*E = 4x1(x2z1^2 - x1)^2*/

    Fp2_sqr(&rop2->x, &tmp6);
    Fp2_sub(&rop2->x, &rop2->x, &tmp5);
    Fp2_sub(&rop2->x, &rop2->x, &tmp7);
    Fp2_sub(&rop2->x, &rop2->x, &tmp7); /* X3 = r^2 - J - 2V = (2y2z1^3 - 2y1)^2 - 4(x2z1^2 - x1)(x2z1^2 - x1)^2 - 8x1(x2z1^2 - x1)^2*/

    Fp2_add(&rop2->z, &T->z, &tmp2);
    Fp2_sqr(&rop2->z, &rop2->z);
    Fp2_sub(&rop2->z, &rop2->z, &tmp11);
    Fp2_sub(&rop2->z, &rop2->z, &tmp3); /* Z3 = (z1 + H)^2 - T1 - I  = 2z1(x2z1^2 - x1) */
    
    Fp2_add(&tmp10, &Q->y, &rop2->z); /* Needed later: tmp10 = y2 + z3*/

    Fp2_sub(&tmp8, &tmp7, &rop2->x);
    Fp2_mul(&tmp8, &tmp8, &tmp6);
    Fp2_mul(&tmp0, &T->y, &tmp5);
    Fp2_mul_u(&tmp0, &tmp0, 2);
    Fp2_sub(&rop2->y, &tmp8, &tmp0); /* Y3 = r(V - X3) - 2Y1*J = (2y2z1^3 - 2y1)(4x1(x2z1^2 - x1)^2 - x3) - 8y1(x2z1^2 - x1)(x2z1^2 - x1)^2*/


    Fp2_sqr(&tmp11, &tmp11); /* T3 = Z3^2 */

    Fp2_sqr(&tmp10, &tmp10); /* tmp10 = (y2 + z3)^2 */
    Fp2_sub(&tmp10, &tmp10, r2);
    Fp2_sub(&tmp10, &tmp10, &tmp11); 
    Fp2_mul_u(&tmp9, &tmp9, 2);
    Fp2_sub(&tmp9, &tmp9, &tmp10); /* tmp9 = 4x2(y2z1^3 - y1) - 2z3y2 */

    //Fp2_mul_fpe(tmp10, rop2->m_z, op3->m_y); /* tmp10 = z3y_Q */
    Fp_mul(&tmp10.x0, &tmp11.x0, &P->y);
    Fp_mul(&tmp10.x1, &tmp11.x1, &P->y);
    Fp2_mul_u(&tmp10, &tmp10, 2);

    Fp2_copy_ne(&tmp6, &tmp6);
    //Fp2_mul_fpe(tmp1, tmp6, op3->m_x);
    Fp_mul(&tmp1.x0, &tmp6.x0, &P->x);
    Fp_mul(&tmp1.x1, &tmp6.x1, &P->x);
    Fp2_mul_u(&tmp1, &tmp1, 2);

    Fp2_set_ui(&tmp2, 0);

    //fp6e_set_fp2e(tfp6e1, tmp2, tmp9, tmp1);
    //fp6e_set_fp2e(tfp6e2, tmp2, tmp2, tmp10);
    
    Fp2_copy(&tfp6e1.x0, &tmp2);
	Fp2_copy(&tfp6e1.x1, &tmp9);
	Fp2_copy(&tfp6e1.x2, &tmp1);
	
	Fp2_copy(&tfp6e2.x0, &tmp2);
	Fp2_copy(&tfp6e2.x1, &tmp2);
	Fp2_copy(&tfp6e2.x2, &tmp10);
	
	Fp6_copy(&f->x0, &tfp6e1);
	Fp6_copy(&f->x1, &tfp6e2);
	
	Fp2_clear(&tmp0);
	Fp2_clear(&tmp1);
	Fp2_clear(&tmp2);
	Fp2_clear(&tmp3);
	Fp2_clear(&tmp4);
	Fp2_clear(&tmp5);
	Fp2_clear(&tmp6);
	Fp2_clear(&tmp7);
	Fp2_clear(&tmp8);
	Fp2_clear(&tmp9);
	Fp2_clear(&tmp10);
	Fp2_clear(&tmp11);
	
	Fp6_clear(&tfp6e1);
	Fp6_clear(&tfp6e2);

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
			f_ltq(&f, &r, &r, Q, P, &r2);
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

void Final_exp(Fp12 *res){
    // First part: (p^6 - 1)
	Fp12 dummy1, dummy2, dummy3, fp, fp2, fp3;
	Fp12_init(&dummy1);
	Fp12_init(&dummy2);
	Fp12_init(&dummy3);
	Fp12_init(&fp);
	Fp12_init(&fp2);
	Fp12_init(&fp3);
	Fp12_copy(&dummy1, res);
	
	// This is exactly the p^6-Frobenius action:
	Fp6_set_ne(&res->x0, &res->x0);
	//fp6e_neg(rop->m_a, rop->m_a);
	
	Fp12_inv(&dummy2, &dummy1);
	Fp12_mul(res, res, &dummy2);
	//fp12e_invert(dummy2, dummy1);
	//fp12e_mul(rop, rop, dummy2);

	// Second part: (p^2 + 1)
	Fp12_copy(&dummy1, res);
	Fp12_frobenius_map_p2(res, res);
	Fp12_mul(res, res, &dummy1);

	// Third part: Hard part (see Implementing cryptographic pairings over BN curves)
	//fp12e_invert(dummy1, rop); // dummy1 = f^{-1}
	Fp12_inv(&dummy1, res);
	
	mpz_t exp;
	mpz_init(exp);
	//mpz_set_str(exp, "114911668330611", 10);
	//mpz_mul_ui(exp, exp, 6);
	//mpz_add_ui(exp, exp, 5);
	
	mpz_pow_ui(exp,prime,4);
	mpz_pow_ui(buf,prime,2);
	mpz_sub(exp,exp,buf);
	mpz_add_ui(exp,exp,1);
	mpz_tdiv_q(exp,exp,order);
	Fp12_exp(ANS,&Tmp,exp);

	Fp12_exp(&dummy2, &dummy1, exp); // dummy2 = f^{-(6x+5)}
	
	/*fp12e_frobenius_p(dummy3, dummy2);
	fp12e_mul(dummy3, dummy3, dummy2); // dummy3 = f^{-(6x+5)p}*f^{-(6x+5)}
	fp12e_frobenius_p(fp, rop);
	fp12e_frobenius_p2(fp2, rop);
	fp12e_frobenius_p(fp3, fp2);
	
	fp12e_square(dummy1, rop);
	fp12e_square(dummy1, dummy1);

	fp12e_mul(rop, rop, fp); // rop = f*f^p

	mpz_set_ui(exp, 9);
	fp12e_pow(rop, rop, exp);
	fp12e_mul(rop, rop, dummy1);
	fp12e_mul(rop, rop, dummy2);
	fp12e_mul(rop, rop, dummy3);
	fp12e_mul(rop, rop, fp3);

	fp12e_square(dummy1, fp);
	fp12e_mul(dummy1, dummy1, fp2);
	fp12e_mul(dummy1, dummy1, dummy3);

	mpz_mul(exp, x, x);
	mpz_mul_ui(exp, exp, 6);
	mpz_add_ui(exp, exp, 1);
	fp12e_pow(dummy1, dummy1, exp);
	fp12e_mul(rop, rop, dummy1);*/
	
	Fp12_clear(&dummy1);
	Fp12_clear(&dummy2);
	Fp12_clear(&dummy3);
	Fp12_clear(&fp);
	Fp12_clear(&fp2);
	Fp12_clear(&fp3);

	mpz_clear(exp);
}


