#include "../include/pairing.h"
#include <stdio.h>
#include <string.h>

int main(void){
    
    EFp P, Q;
    Fp private1, private2;
    Fp12 res;
    Fp12_init(&res);
    
    mpz_init(prime);
	mpz_set_str(prime, "6277099611578052373965416350240300935678131109448722797563", 10);
    EFp_init(&P);
    EFp_init(&Q);
    
    mpz_set_str(P.x.x0, "1", 10);
    mpz_set_str(P.z.x0, "1", 10);
    mpz_set_str(P.y.x0, "3120702393732050978281592811936274336057498953844400890961", 10);
    
    mpz_set_str(Q.x.x0, "1", 10);
    mpz_set_str(Q.z.x0, "1", 10);
    mpz_set_str(Q.y.x0, "3120702393732050978281592811936274336057498953844400890961", 10);
    
    Fp_init(&private1);
    Fp_init(&private2);
    
    //private2.x0[5] = 1;
    //EFp_get_generator(&P);
    //EFp_get_generator(&Q);
	
	Fp_get_random(&private1);
	Fp_get_random(&private2);
	
	Fp_set_ui(&private1, 3);
	Fp_set_ui(&private2, 4);
	
	EFp_mul(&P, private1.x0, &P);
	EFp_mul(&Q, private2.x0, &Q);
	EFp_to_affine(&P);
	EFp_to_affine(&Q);
	
	sym_ate_paring(&res, &P, &Q);
    
    printf("\n");
    
    
    
    EFp_clear(&P);
    EFp_clear(&Q);  
    Fp12_clear(&res);
    return 0;
}
