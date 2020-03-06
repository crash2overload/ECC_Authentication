int ecc_get_Signatur(Curve_params_t * curve, Node_t *node, const void *msg, 
						int msg_len, void *r, void *s, int hash_len, int dd);
int ecc_verfi_Signatur(Curve_params_t * curve, Node_t *node, const void *msg, 
						int msg_len, void *r, void *s, int hash_len, int dd);
void get_Hash(const void *msg, int msg_len, void *hash, int hash_len);
