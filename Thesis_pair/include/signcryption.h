#ifndef CRYPTO
#define CRYPTO
#include "crypto.h"
#endif

int Signcryption (Curve_params_t *curve, Node_t *node, Point_t *publicKey, void *msg, size_t msg_len, void *r, void *s, int dd);
int Unsigncryption (Curve_params_t *curve, Node_t *node, Point_t *publicKey, void *msg, size_t msg_len, void *r, void *s, int dd);
