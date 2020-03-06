
#include <stdlib.h>
#include <linux/types.h>
#include <asm-generic/fcntl.h>
#include <asm-generic/fcntl.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <inttypes.h>
#include <linux/ioctl.h>
#include <string.h>
#include <stdio.h>

#include "../include/curves.h"
#include "../include/node.h"

void init_node(Curve_params_t *curve, Node_t *node){
	node->publicKey = malloc(curve->prec/4);
	node->privateKey = malloc(curve->prec/8);
	node->sharedKey = malloc(curve->prec/8);
	node->Point.x = malloc(curve->prec/8);
	node->Point.y = malloc(curve->prec/8);
	node->Point.z = malloc(curve->prec/8);
}

void init_Point(Curve_params_t *curve, Point_t *Point){
	Point->x = malloc(curve->prec/8);
	Point->y = malloc(curve->prec/8);
	Point->z = malloc(curve->prec/8);
}

void free_node(Node_t *node){
	free(node->publicKey);
	free(node->privateKey);
	free(node->sharedKey);
	free(node->Point.x);
	free(node->Point.y);
	free(node->Point.z);
}

void free_Point(Point_t *Point){
	free(Point->x);
	free(Point->y);
	free(Point->z);
}
