#ifndef CRYPTO
#define CRYPTO
#include "crypto.h"
#endif

int ecc_Point_Add(Curve_params_t * curve, Point_t *PointP, Point_t *PointQ, int dd);
int ecc_Point_Double(Curve_params_t * curve, Point_t *PointP, int dd);
int ecc_Point_Multi(Curve_params_t * curve, __u32 *skalar, Point_t *PointP, int dd);
int ecc_Make_Key(Curve_params_t * curve, Node_t *node, int dd);
int make_random_Key(__u32 *privateKey, int Precision, int dd);
