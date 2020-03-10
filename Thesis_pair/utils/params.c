//#include "../include/params.h"
#include "../include/efp.h"

int main(void){
	init_params();
	mpz_set(prime, p);
	gmp_printf ("u: %Zd\n", u);
	gmp_printf ("t: %Zd\n", t);
	gmp_printf ("p: %Zd\n", p);
	
	gmp_printf ("order: %Zd\n", n);
	gmp_printf ("zeta: %Zd\n", zeta);
	Fp2_print(&bt);
	EFp2 P;
	EFp2_init(&P);
	EFp2_set_generator(&P);
	EFp2_printf(&P, "Generator");
	EFp2_clear(&P);
	clear_params();
	return 0;
}

void init_params(){
	//Set u
	mpz_init(u);
	mpz_set_str(u, "11000000000000000001000000000000000010000010011", 2);
	//mpz_set_str(u, "6917529027641089837", 10);
	// Generate trace
	mpz_init(t);
	mpz_mul(t, u, u);
	mpz_mul_ui(t, t, 6);
	mpz_add_ui(t, t, 1);
	// Calculate p
	mpz_t buf;
    mpz_init(buf);
    mpz_init(p);
    mpz_pow_ui(buf, u, 4);
    mpz_mul_ui(buf, buf, 36);
    mpz_set(p, buf);
    mpz_pow_ui(buf, u, 3);
    mpz_mul_ui(buf, buf, 36);
    mpz_add(p, p, buf);
    mpz_pow_ui(buf, u, 2);
    mpz_mul_ui(buf, buf, 24);
    mpz_add(p, p, buf);
    mpz_mul_ui(buf, u, 6);
    mpz_add(p, p, buf);
    mpz_add_ui(p, p, 1);
    
    // Calulate order
    mpz_init(n);
    /* See https://cryptojedi.org/papers/fast-bn-20110603.pdf */
    mpz_add_ui(n, p, 1);
    mpz_sub(n, n, t);
    
    //Set Coeffizient b
    b = 3;
    
    // Calculate zeta
    mpz_init(zeta);
    mpz_pow_ui(buf, u, 3);
    mpz_mul_ui(buf, buf, 18);
    mpz_set(zeta, buf);
    mpz_pow_ui(buf, u, 2);
    mpz_mul_ui(buf, buf, 18);
    mpz_add(zeta, zeta, buf);
    mpz_mul_ui(buf, u, 9);
    mpz_add(zeta, zeta, buf);
    mpz_add_ui(zeta, zeta, 1);
    /******** Preperation for Curve in Fp2 ****************/
    Fp2_init(&Fp2_0);
	Fp2_init(&Fp2_1);
	Fp2_init(&Fp2_i);
	Fp2_init(&bt);
	
	Fp2_set_ui(&Fp2_0, 0);
	Fp_set_ui(&Fp2_1.x0, 1);
	Fp_set_ui(&Fp2_i.x0, 0);
	Fp_set_ui(&Fp2_i.x0, 1);
	/***TODO***/
	Fp_set_ui(&bt.x0, b);
	Fp_set_ui(&bt.x1, b);
	
	mpz_init(ht);
	mpz_sub_ui(ht, p, 1);
	mpz_add(ht, ht, t);
	
    
    mpz_clear(buf);
}

void clear_params(){
	mpz_clear(u);
	mpz_clear(t);
	mpz_clear(p);
	mpz_clear(n);
	mpz_clear(zeta);
	Fp2_clear(&Fp2_0);
	Fp2_clear(&Fp2_1);
	Fp2_clear(&Fp2_i);
	Fp2_clear(&bt);
	mpz_clear(ht);

}
