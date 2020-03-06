
int Fp_mult_core(__u32 *res, __u32 *value1, __u32 *value2);
int Fp_inv_core(__u32 *element);
int Fp_exp_core(__u32 *res, __u32 *element, __u32 *skalar);
int Fp_add_core(__u32 *res, __u32 *value1, __u32 *value2);
int Fp_sub_core(__u32 *res, __u32 *value1, __u32 *value2);
int EFp_mul_core(__u32 *skalar, EFp *P);
int EFp_add_core(EFp *res, EFp *P, EFp *Q);
