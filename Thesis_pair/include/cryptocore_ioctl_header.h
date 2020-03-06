/*
* cryptocore_ioctl_header.h - the header file with the ioctl definitions.
* The declarations here have to be in a header file, because
* they need to be known both the kernel module in *_driver.c
* and the application *_app.c
*/

#include <linux/ioctl.h>
//#include "curves.h"

// CryptoCore Operations:

#define MONTMULT			0x0
#define MONTR				0x1
#define MONTR2				0x2
#define MONTEXP				0x3
#define MODADD				0x4
#define MODSUB				0x5
#define COPYH2V				0x6
#define COPYV2V				0x7
#define COPYH2H				0x8
#define COPYV2H				0x9
#define MONTMULT1			0xA
#define MONTEXPFULL			0xB
#define ECC_POINT_ADD		0xC
#define ECC_POINT_DOUBLE	0xD
#define ECC_POINT_MULTI		0xE
#define DEJACOBIAN			0xF
#define INVERT_ELEM			0x10
#define VERIFICATION		0x11
#define SIGNCRYPTION		0x12
#define MONTGOMERIZE		0x13

// CryptoCore RAM Locations

#define MWMAC_RAM_B				0x0
#define MWMAC_RAM_P				0x1
#define MWMAC_RAM_TS			0x2
#define MWMAC_RAM_TC			0x3
#define MWMAC_RAM_A				0x4
#define MWMAC_RAM_E				0x5
#define MWMAC_RAM_X				0x6

#define MWMAC_RAM_B1 			0x0
#define MWMAC_RAM_B2 			0x1
#define MWMAC_RAM_B3 			0x2
#define MWMAC_RAM_B4 			0x3
#define MWMAC_RAM_B5 			0x4
#define MWMAC_RAM_B6 			0x5
#define MWMAC_RAM_B7 			0x6
#define MWMAC_RAM_B8 			0x7

#define MWMAC_RAM_P1			0x8
#define MWMAC_RAM_P2			0x9
#define MWMAC_RAM_P3			0xA
#define MWMAC_RAM_P4			0xB
#define MWMAC_RAM_P5			0xC
#define MWMAC_RAM_P6			0xD
#define MWMAC_RAM_P7			0xE
#define MWMAC_RAM_P8			0xF

#define MWMAC_RAM_TS1			0x10
#define MWMAC_RAM_TS2			0x11
#define MWMAC_RAM_TS3			0x12
#define MWMAC_RAM_TS4			0x13
#define MWMAC_RAM_TS5			0x14
#define MWMAC_RAM_TS6			0x15
#define MWMAC_RAM_TS7			0x16
#define MWMAC_RAM_TS8			0x17

#define MWMAC_RAM_TC1			0x18
#define MWMAC_RAM_TC2			0x19
#define MWMAC_RAM_TC3			0x1A
#define MWMAC_RAM_TC4			0x1B
#define MWMAC_RAM_TC5			0x1C
#define MWMAC_RAM_TC6			0x1D
#define MWMAC_RAM_TC7			0x1E
#define MWMAC_RAM_TC8			0x1F

#define MWMAC_RAM_A1			0x0
#define MWMAC_RAM_A2			0x1
#define MWMAC_RAM_A3			0x2
#define MWMAC_RAM_A4			0x3
#define MWMAC_RAM_A5			0x4
#define MWMAC_RAM_A6			0x5
#define MWMAC_RAM_A7			0x6
#define MWMAC_RAM_A8			0x7

#define MWMAC_RAM_E1			0x8
#define MWMAC_RAM_E2			0x9
#define MWMAC_RAM_E3			0xA
#define MWMAC_RAM_E4			0xB
#define MWMAC_RAM_E5			0xC
#define MWMAC_RAM_E6			0xD
#define MWMAC_RAM_E7			0xE
#define MWMAC_RAM_E8			0xF

#define MWMAC_RAM_X1			0x10
#define MWMAC_RAM_X2			0x11
#define MWMAC_RAM_X3			0x12
#define MWMAC_RAM_X4			0x13
#define MWMAC_RAM_X5			0x14
#define MWMAC_RAM_X6			0x15
#define MWMAC_RAM_X7			0x16
#define MWMAC_RAM_X8			0x17

// CryptoCore Struct Declarations:

typedef struct ModExp_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[128];
	__u32 b[128];
	__u32 e[128];
	__u32 c[128];
} ModExp_params_t;

typedef struct ModRed_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[64];
	__u32 a[128];
	__u32 c[128];
} ModRed_params_t ;

// Add CryptoCore Struct Declarations here...

typedef struct Core_CMD{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 cmd;
	__u32 B[128];
	__u32 P[128];
	__u32 TS[128];
	__u32 TC[128];
	__u32 X[128];
	__u32 E[128];
	__u32 A[128];
	__u32 src;
	__u32 dest;
}Core_CMD_t;
/*
typedef struct Point{
	__u32 x[64];
	__u32 y[64];
	__u32 z[64];
}Point_t;

typedef struct Node{
	Point_t poin;
	__u32 privateKey[64];
	__u32 publicKey[128];
	__u32 sharedKey[64];
}Node_t;*/






#define IOCTL_BASE 'k' 					// magic number

// NOTE: magic | cmdnumber | size of data to pass
#define 	IOCTL_SET_TRNG_CMD			_IOW(IOCTL_BASE,   1, __u32)
#define 	IOCTL_SET_TRNG_CTR			_IOW(IOCTL_BASE,   2, __u32)
#define 	IOCTL_SET_TRNG_TSTAB		_IOW(IOCTL_BASE,   3, __u32)
#define 	IOCTL_SET_TRNG_TSAMPLE		_IOW(IOCTL_BASE,   4, __u32)
#define 	IOCTL_READ_TRNG_FIFO		_IOR(IOCTL_BASE,   5, __u32)

#define		IOCTL_MWMAC_MONTMULT		_IOWR(IOCTL_BASE,  6, Core_CMD_t)
#define		IOCTL_MWMAC_MONTR			_IOWR(IOCTL_BASE,  7, Core_CMD_t)
#define		IOCTL_MWMAC_WRITE_RAM		_IOWR(IOCTL_BASE,  8, Core_CMD_t)
#define		IOCTL_MWMAC_MONTEXP			_IOWR(IOCTL_BASE,  9, Core_CMD_t)
#define		IOCTL_MWMAC_MODADD			_IOWR(IOCTL_BASE, 10, Core_CMD_t)
#define		IOCTL_MWMAC_MODSUB			_IOWR(IOCTL_BASE, 11, Core_CMD_t)
#define		IOCTL_MWMAC_COPY			_IOWR(IOCTL_BASE, 12, Core_CMD_t)
#define		IOCTL_MWMAC_CLEAR			_IOWR(IOCTL_BASE, 13, Core_CMD_t)
#define		IOCTL_MWMAC_READ_RAM		_IOWR(IOCTL_BASE, 14, Core_CMD_t)
#define		IOCTL_MWMAC_ECC_POINT_ADD	_IOWR(IOCTL_BASE, 15, Core_CMD_t)
#define		IOCTL_MWMAC_MONTMULT1		_IOWR(IOCTL_BASE, 16, Core_CMD_t)
#define		IOCTL_MWMAC_MONTEXP_FULL	_IOWR(IOCTL_BASE, 17, Core_CMD_t)
#define		IOCTL_MWMAC_MODEXP			_IOWR(IOCTL_BASE, 18, ModExp_params_t)
#define		IOCTL_MWMAC_MODRED			_IOWR(IOCTL_BASE, 19, ModRed_params_t)
#define		IOCTL_MWMAC_ECC_POINT_DOUBLE	_IOWR(IOCTL_BASE, 20, Core_CMD_t)
#define		IOCTL_MWMAC_ECC_POINT_MULTIPLICATION	_IOWR(IOCTL_BASE, 21, Core_CMD_t)
#define 	IOCTL_MWMAC_DEJACOBIAN		_IOWR(IOCTL_BASE, 22, Core_CMD_t)
#define 	IOCTL_MWMAC_INVERT_ELEM		_IOWR(IOCTL_BASE, 23, Core_CMD_t)
#define 	IOCTL_MWMAC_VERIFICATION	_IOWR(IOCTL_BASE, 24, Core_CMD_t)
#define 	IOCTL_MWMAC_SIGNCRYPTION	_IOWR(IOCTL_BASE, 25, Core_CMD_t)
#define 	IOCTL_MWMAC_MONTGOMERIZE	_IOWR(IOCTL_BASE, 26, Core_CMD_t)

// Define further IOCTL commands here...
