#ifndef CRYPTO
#define CRYPTO
#include "crypto.h"
#endif

int affine_to_jacobian(Curve_params_t *curve, Node_t *node, int dd);
int affine_to_jacobian2(Curve_params_t *curve, Point_t *Point, int dd);
int jacobian_to_affine(Curve_params_t *curve, Node_t *node, int dd);
int jacobian_to_affine2(Curve_params_t *curve, Point_t *P, int dd);
