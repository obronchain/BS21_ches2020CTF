#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "clyde_masked.h"
#include "primitives.h"
#include "prng.h"
int main(){
    clyde128_state nonce;
    clyde128_state tk;
    clyde128_masked_state k;
    uint32_t seed[4];

    while(1){
        if(read(STDIN_FILENO,seed,16)<16){exit(0);}
        if(read(STDIN_FILENO,nonce,16)<16){exit(-1);}
        if(read(STDIN_FILENO,tk,16)<16){exit(-1);};
        if(read(STDIN_FILENO,k,D*16)<D*16){exit(-1);}
        init_rng(seed);
        clyde128_encrypt(nonce,tk,k);
    }

}
