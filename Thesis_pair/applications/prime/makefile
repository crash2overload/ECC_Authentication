LDFLAGS = -lrt
USEPKG += -lb2
LDFLAGS2 = -lpbc -lgmp

CC = gcc

signcryption:	SIGNCRYPTION_FPGA.o mont_util.o jacobian.o ecc_util.o signcryption.o aes.o cryptocore_util.o node.o
	$(CC) -o SIGNCRYPTION_FPGA SIGNCRYPTION_FPGA.o mont_util.o jacobian.o ecc_util.o signcryption.o aes.o cryptocore_util.o node.o $(LDFLAGS) $(USEPKG) -Wall
	
ECDH:	ECDH.o mont_util.o jacobian.o ecc_util.o cryptocore_util.o node.o
	$(CC) -o ECDH ECDH.o mont_util.o jacobian.o ecc_util.o cryptocore_util.o node.o $(LDFLAGS)

ECDSA_FPGA:	ECDSA_FPGA.o mont_util.o jacobian.o ecc_util.o cryptocore_util.o node.o
	$(CC) -o ECDSA_FPGA ECDSA_FPGA.o mont_util.o jacobian.o ecc_util.o cryptocore_util.o node.o $(LDFLAGS)

ECDH_3_Party:	ECDH_FPGA_3.o mont_util.o jacobian.o ecc_util.o cryptocore_util.o node.o
	$(CC) -o ECDH_3_Party ECDH_FPGA_3.o mont_util.o jacobian.o ecc_util.o cryptocore_util.o node.o $(LDFLAGS)

ECHD_PAIRING: ECDH_PAIRING.o
	$CC -o ECDH_PAIRING ECHD_PAIRING.o -lpbc -lgmp

mont_util.o:	../../utils/mont_util.c
		$(CC) -c ../../utils/mont_util.c

aes.o:	../../utils/aes.c
		$(CC) -c ../../utils/aes.c

jacobian.o: 	../../utils/jacobian.c
		$(CC) -c ../../utils/jacobian.c

ecc_util.o: 	../../utils/ecc_util.c
		$(CC) -c ../../utils/ecc_util.c

cryptocore_util.o: 	../../utils/cryptocore_util.c
		$(CC) -c ../../utils/cryptocore_util.c

ecc_signatur.o:	../../utils/ecc_signatur.c
		$(CC) -c ../../utils/ecc_signatur.c

		
node.o:	../../utils/node.c
		$(CC) -c ../../utils/node.c

signcryption.o:	../../utils/signcryption.c
		$(CC) -c ../../utils/signcryption.c

SIGNCRYPTION_FPGA.o: SIGNCRYPTION_FPGA.c
		gcc -c SIGNCRYPTION_FPGA.c -Wall

ECDH.o:	ECDH.c
		$(CC) -c ECDH.c

ECDH_FPGA_dynamisch.o:	ECDH_FPGA_dynamisch.c
		$(CC) -c ECDH_FPGA_dynamisch.c

ECDSA_FPGA.o:	ECDSA_FPGA.c
		$(CC) -c ECDSA_FPGA.c

ECDH_FPGA_3.o:	ECDH_FPGA_3.c
		$(CC) -c ECDH_FPGA_3.c

ECDH_PAIRING.o:	ECDH_PAIRING.c
		$(CC) -c ECDH_PAIRING.c

clean:
	rm -f *.o
