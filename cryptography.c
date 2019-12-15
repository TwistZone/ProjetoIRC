//
// Created by francisco on 14/12/2019.
//

#include <sodium.h>
#include <string.h>

int main() {
    //working example
    //Must assume all parties have the key, so we share it through a file PSK
    //Must send over the encrypted content, the nonce and the encrypted content's size

    unsigned char key[crypto_secretbox_KEYBYTES];
    FILE *fp;


    //init library
    if (sodium_init() < 0) {
        printf("Sodium library failed, aborting\n");
        exit(-1);
    }

    fp = fopen("PSK", "rb");
    if (!fp) {
        printf("generating key\n");
        //generate 256 bit key (32*8 = 256)
        crypto_secretbox_keygen(key);
        fp = fopen("PSK", "wb");
        fwrite(key, 1, crypto_secretbox_KEYBYTES, fp);
        fclose(fp);
    } else {
        printf("getting key\n");
        fread(key, 1, crypto_secretbox_KEYBYTES, fp);
        fclose(fp);
    }

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof(nonce));
    //set a string
    unsigned char string[1000] = "Isto e um belo teste de encriptacao";
    unsigned char encrypted[1000];

    printf("original:\n%s\n", string);
    int original_size = (int) strlen(string);
    //encrypt
    crypto_secretbox_easy(encrypted, string, original_size, nonce, key);
    printf("encriptada:\n%s\n", encrypted);
    *string = 0;
    //decrypt
    int erro = crypto_secretbox_open_easy(string, encrypted, crypto_secretbox_MACBYTES + original_size, nonce, key);
    printf("desencriptada:\n%s\n", string);
    return 0;
}