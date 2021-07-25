#include "security.h"
#include <stdio.h>
//#include <stdlib.h>
#include <string.h>

#include "des.h"

void DES3(unsigned char *data, unsigned char **key, int mode) {
    key_set key_sets1[17];
    key_set key_sets2[17];
    key_set key_sets3[17];
    unsigned char data_block2[8];
    unsigned char data_block3[8];

    generate_sub_keys(key[0], key_sets1);
    generate_sub_keys(key[1], key_sets2);
    generate_sub_keys(key[2], key_sets3);

    if(mode) { // encrypt
        process_message(data, data_block2, key_sets1, 1);
        process_message(data_block2, data_block3, key_sets2, 0);
        process_message(data_block3, data, key_sets3, 1);
    }
    else{
        process_message(data, data_block2, key_sets1, 0);
        process_message(data_block2, data_block3, key_sets2, 1);
        process_message(data_block3, data, key_sets3, 0);
    }
}

void digest(unsigned char *data, unsigned char* key, unsigned char *final){
    SHA256_CTX x1;
    unsigned char new_data[24];

    // Digest fatto su K | M | K
    int i=0;
    for(i=0; i<8; i++)
        new_data[i] = key[i];
    for(i=8; i<16; i++)
        new_data[i] = data[i];
    for(i=16; i<24; i++)
        new_data[i] = key[i];

    sha256_init(&x1);
    sha256_update(&x1, new_data, 24);
    sha256_final(&x1, final);

}

int verify_digest(unsigned char *data, unsigned char* key, unsigned char* dig){
    unsigned char final[32];
		int i;
    digest(data, key, final);
		

    for(i = 0; i<32; i++){
        if(final[i] != dig[i])
            return 0;
    }

    return 1;
}

void AES(struct AES_ctx* ctx, uint8_t *data){
    AES_CTR_xcrypt_buffer(ctx, data, 16);
}

struct AES_ctx* AES_init(const uint8_t *key, const uint8_t iv){
    struct AES_ctx* aes;
    AES_init_ctx_iv(aes, key, iv);
    return struct aes;
}
