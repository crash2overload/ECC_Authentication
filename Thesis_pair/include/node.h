//#ifndef CRYPTO
//#define CRYPTO
//#include "crypto.h"
//#endif

//#include "curves.h"

typedef struct {
	__u32 *x;
	__u32 *y;
	__u32 *z;
}Point_t;

typedef struct Node{
	Point_t Point;
	__u32 *privateKey;
	__u32 *publicKey;
	__u32 *sharedKey;
}Node_t;

void init_node(Curve_params_t *curve, Node_t *node);
void init_Point(Curve_params_t *curve, Point_t *Point);
void free_node(Node_t *node);
void free_Point(Point_t *Point);
