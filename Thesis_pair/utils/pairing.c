#include "../include/pairing.h"
#include <stdio.h>
#include <gmp.h>

void get_frobenius(){
	Fp2 tmp1,tmp2,tmp3, Alpha_1;
	Fp2_init(&tmp1);
	Fp2_init(&tmp2);
	Fp2_init(&tmp3);
	Fp2_init(&Alpha_1);
	
	Fp2_set_ui(&Alpha_1, 1);
	mpz_t exp,buf,p2;
	mpz_init(exp);
	mpz_init(buf);
	mpz_init(p2);
	int i,j;
	for(i=0; i<12; i++){
        for(j=0; j<6; j++){
            Fp2_init(&frobenius_constant[i][j]);
        }
	}
	
	//mpz_set_str(p2,"6277099611578052373965416350240300935678131109448722797563", 10);
	
	mpz_mul(p2,prime,prime);
	//frobenius_2
	mpz_sub_ui(exp,p2,1);
	mpz_tdiv_q_ui(exp,exp,3);
	
	Fp2_exp(&tmp1,&Alpha_1,exp);
	Fp2_mul(&tmp2,&tmp1,&tmp1);
	mpz_tdiv_q_ui(exp,exp,2);
	Fp2_exp(&tmp3,&Alpha_1,exp);
	
	//set f_p2
	Fp_set_ui(&frobenius_constant[0][0].x0,1);
	Fp2_copy(&frobenius_constant[0][1],&tmp1);
	Fp2_copy(&frobenius_constant[0][2],&tmp2);
	Fp2_copy(&frobenius_constant[0][3],&tmp3);
	Fp2_mul(&frobenius_constant[0][4],&tmp1,&tmp3);
	Fp2_mul(&frobenius_constant[0][5],&tmp2,&tmp3);
	
	Fp2_clear(&tmp1);
	Fp2_clear(&tmp2);
	Fp2_clear(&tmp3);
	mpz_clear(exp);
	mpz_clear(buf);
	mpz_clear(p2);
}

void sym_ate_paring(Fp12 *res, EFp *P, EFp *Q){
	
    Fp12 test1,test2,test3;

	EFp2 PP;
	EFp2_init(&PP);
	EFp2_rational_point(&PP);
	EFp2_printf(&PP, "PP");
    Fp12_init(&test1);
    Fp12_init(&test2);
    Fp12_init(&test3);
    
    EFp_print(P);
    printf("\n");
    EFp_print(Q);
    
	get_frobenius();
	printf("\n test1 \n");
    Miller_ate(res, &PP, P);
    
    
    Final_exp(res);
    Fp12_print(res);
    /*printf("\n test2 \n");
    Miller_ate(res, Q, P);
	
	Final_exp(res, res);
	
	Fp12_print(res);*/
       
    Fp12_clear(&test1);
    Fp12_clear(&test2);
    Fp12_clear(&test3);
	
}

//twist
void EFp12_to_EFp2(EFp2 *res, EFp12 *A){
    Fp2_copy(&res->x, &A->x.x0.x1);
    Fp2_copy(&res->y, &A->y.x1.x1);
}

void EFp2_to_EFp12(EFp12 *res, EFp2 *A){
    Fp2_copy(&res->x.x0.x1, &A->x);
    Fp2_copy(&res->y.x1.x1, &A->y);
}

void EFp12_to_EFp(EFp *res, EFp12 *A){
    Fp_copy(&res->x, &A->x.x0.x0.x0);
    Fp_copy(&res->y, &A->y.x0.x0.x0);
}

void EFp_to_EFp12(EFp12 *res, EFp *A){
    Fp_copy(&res->x.x0.x0.x0, &A->x);
    Fp_copy(&res->y.x0.x0.x0, &A->y);
}

void EFp2_to_EFp(EFp *res,EFp2 *A){
	Fp_copy(&res->x, &A->x.x0);
    Fp_copy(&res->y, &A->y.x0);
}

void EFp_to_EFp2(EFp2 *res,EFp *A){
	Fp_copy(&res->x.x0, &A->x);
	Fp_copy(&res->x.x1, &A->x);
	Fp_copy(&res->y.x0, &A->y);
    Fp_copy(&res->y.x1, &A->y);
}

//void Fp2_pow(Fp2 *ANS,Fp2 *A,mpz_t scalar){
    //int i,length;
    //length=(int)mpz_sizeinbase(scalar,2);
    //char binary[length];
    //mpz_get_str(binary,2,scalar);
    //Fp2 tmp;
    //Fp2_init(&tmp);
    //Fp2_copy(&tmp,A);
    
    //for(i=1; binary[i]!='\0'; i++){
        //Fp2_sqr(&tmp,&tmp);
        //if(binary[i]=='1'){
            //Fp2_mul(&tmp,A,&tmp);
        //}
    //}
    
    //Fp2_copy(ANS,&tmp);
    //Fp2_clear(&tmp);
//}




