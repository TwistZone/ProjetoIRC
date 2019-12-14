//
// Created by francisco on 14/12/2019.
//

#include <sodium.h>
#include <string.h>

int main() {
    unsigned char key[crypto_secretbox_KEYBYTES];
    //init library
    if (sodium_init() < 0) {
        printf("Sodium library failed, aborting\n");
        exit(-1);
    }

    //generate 256 bit key (32*8 = 256)
    crypto_secretbox_keygen(key);
    FILE *fp = fopen("PSK", "wb");
    fwrite(key, 1, 32, fp);
    fclose(fp);

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof(nonce));
    //memset(nonce, 0, sizeof(nonce));
    //set a string
    unsigned char string[1000] = "Isto e um belo teste de encriptacao";
    unsigned char encrypted[1000];

    printf("original:\n%s\n", string);
    int a = strlen(string);
    //encrypt
    crypto_secretbox_easy(encrypted, string, a, nonce, key);
    printf("encriptada:\n%s\n", encrypted);
    *string = 0;
    //decrypt
    crypto_secretbox_open_easy(string, encrypted, crypto_secretbox_MACBYTES + a, nonce, key);
    printf("desencriptada:\n%s\n", string);
    return 0;
}