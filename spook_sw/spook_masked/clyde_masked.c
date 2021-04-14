/*
 * Copyright 2020 UCLouvain
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "clyde_masked.h"
#include "parameters.h"
#include "utils_masking.h"
#include "primitives.h"

#define REFRESH_STATE(STATE) do { \
    refresh(&(STATE)[0]); \
    refresh(&(STATE)[1*D]); \
    refresh(&(STATE)[2*D]); \
    refresh(&(STATE)[3*D]);} while (0)

#define DUMP_LAB(LAB,ID,DATA) do{ \
    fprintf(stdout,"%s_%d\n%08x\n",(LAB),(ID),(DATA));\
}while(0);

#define DUMP_ENC(LAB,ENC) do{\
    for(int i=0;i<D;i++){DUMP_LAB(LAB,i,(ENC)[i]);}; \
}while(0);


/*
* func: clyde128_encrypt
* performs Clyde encryption of state with a tweat t. The input key is shared
* and there is a table of uin32_t of length 4*D.
*/
void clyde128_encrypt(clyde128_state state, const clyde128_state t, uint32_t *k) {
    clyde128_state tk[3] = {
		{ t[0], t[1], t[2], t[3] },
		{ t[0] ^ t[2], t[1] ^ t[3], t[0], t[1] },
		{ t[2], t[3], t[0] ^ t[2], t[1] ^ t[3] }
    };
    // init the masked state
    clyde128_masked_state masked_state,key_state;
    local_memcpy(key_state,k,4*D);
    local_memcpy(masked_state,key_state,4*D);

    // add the key
    XORLS_MASK(masked_state,tk[0]);
    XORLS_MASK(masked_state,state);

    uint32_t off = 0x924;		// 2-bits describing the round key
    uint32_t lfsr = 0x8;

    // Datapath
    for (uint32_t s = 0; s < 6; s++) {
        // sbox and lbox layers
        fprintf(stdout,"=");
        sbox_layer_masked(&masked_state[0*D],&masked_state[1*D],&masked_state[2*D],&masked_state[3*D],1);
        #ifdef ROUNDREDUCED
        break;
        #endif


        lbox_masked(masked_state);

        // constant addition
        XORCST_MASK(masked_state,lfsr);
        uint32_t b = lfsr & 0x1;
        lfsr = (lfsr^(b<<3) | b<<4)>>1;	// update LFSR

        // sbox and lbox layers
        sbox_layer_masked(&masked_state[0*D],&masked_state[1*D],&masked_state[2*D],&masked_state[3*D],0);
        lbox_masked(masked_state);
        // constant addition
        XORCST_MASK(masked_state,lfsr);
        b = lfsr & 0x1;
        lfsr = (lfsr^(b<<3) | b<<4)>>1; // update LFSR

        // key addition
        add_clyde128_masked_state(masked_state,masked_state,key_state);
        // tweak addition
        off >>=2;
        XORLS_MASK(masked_state,tk[off&0x3]);

    }

    //  copy refreshed_key
    REFRESH_STATE(key_state);
    local_memcpy(k,key_state,4*D);

    state[0] = unmask(&masked_state[0]);
    state[1] = unmask(&masked_state[1*D]);
    state[2] = unmask(&masked_state[2*D]);
    state[3] = unmask(&masked_state[3*D]);
}

void clyde128_decrypt(clyde128_state state, const clyde128_state t, uint32_t *k) {
    clyde128_state tk[3] = {
		{ t[0], t[1], t[2], t[3] },
		{ t[0] ^ t[2], t[1] ^ t[3], t[0], t[1] },
		{ t[2], t[3], t[0] ^ t[2], t[1] ^ t[3] }
    };

    // init the masked state
    clyde128_masked_state masked_state,key_state;
    memset(masked_state,0,16*D);
    local_memcpy_bytes(key_state,k,4*D*sizeof(uint32_t));

    // set state into masked_state
    XORLS_MASK(masked_state,state);


    // Datapath
    uint32_t off = 0x618;		// 2-bits describing the round key
    uint32_t lfsr = 0x7;
    for (uint32_t s = 0; s < 6; s++) {
        // tweak addition
        XORLS_MASK(masked_state,tk[off&0x3]);
        // key addition
        add_clyde128_masked_state(masked_state,masked_state,key_state);
        // round constant addition
        XORCST_MASK(masked_state,lfsr);

        // inversed lbox and sbox layers
        lbox_inv_masked(masked_state);
        sbox_inv_layer_masked(&masked_state[0*D],&masked_state[1*D],&masked_state[2*D],&masked_state[3*D]);

        // round constant addition
        uint32_t b = lfsr >> 3;
        lfsr = ((lfsr << 1) | b) ^ (b<<3);	// update LFSR
        lfsr &= 0xf;
        XORCST_MASK(masked_state,lfsr);

        // inversed lbox and sbox layer
        lbox_inv_masked(masked_state);
        sbox_inv_layer_masked(&masked_state[0*D],&masked_state[1*D],&masked_state[2*D],&masked_state[3*D]);

        b = lfsr >> 3;
        lfsr = ((lfsr << 1) | b) ^ (b<<3);
        lfsr &= 0xf;
        off >>=2;

        #ifdef ROUNDREDUCED
        break;
        #endif
    }

    // add the key
    XORLS_MASK(masked_state,tk[0]);
    add_clyde128_masked_state(masked_state,masked_state,key_state);
    state[0] = unmask(&masked_state[0]);
    state[1] = unmask(&masked_state[1*D]);
    state[2] = unmask(&masked_state[2*D]);
    state[3] = unmask(&masked_state[3*D]);

    //  copy refreshed_key
    REFRESH_STATE(key_state);
    local_memcpy(k,key_state,4*D);
}


// Apply a S-box layer to a Clyde-128 state.
void sbox_layer_masked(uint32_t* a,uint32_t *b, uint32_t *c, uint32_t *d,uint32_t refresh_flag) {
    uint32_t y0[D],y1[D],y3[D],tmp[D];
    DUMP_ENC("a",a);
    DUMP_ENC("b",b);
    DUMP_ENC("c",c);
    DUMP_ENC("d",d);
    mult_shares(tmp,a,b);
    DUMP_ENC("tmp0",tmp);
    add_shares(y1,tmp,c);
    DUMP_ENC("y1",y1);
    if(refresh_flag){
        refresh(y1);
        DUMP_ENC("y1r",y1);
    }

    mult_shares(tmp,d,a);
    DUMP_ENC("tmp1",tmp);
    add_shares(y0,tmp,b);
    DUMP_ENC("y0",y0);

    mult_shares(tmp,y1,d);
    DUMP_ENC("tmp2",tmp);
    add_shares(y3,tmp,a);
    DUMP_ENC("y3",y3);

    mult_shares(tmp,y0,y1);
    DUMP_ENC("tmp3",tmp);
    add_shares(c,tmp,d);
    DUMP_ENC("y2",c);

    local_memcpy(a,y0,D);
    local_memcpy(b,y1,D);
    local_memcpy(d,y3,D);
}

void lbox_masked(clyde128_masked_state masked_state){
    for(uint32_t i=0;i<D;i++){
        lbox_protected(&masked_state[0*D + i],&masked_state[1*D +i]);
        lbox_protected(&masked_state[2*D + i],&masked_state[3*D +i]);
    }
}


// Apply a S-box layer to a Clyde-128 state.
void sbox_inv_layer_masked(uint32_t* a,uint32_t *b, uint32_t *c, uint32_t *d) {
    uint32_t y0[D],y1[D],y3[D],tmp[D];

    mult_shares(tmp,a,b);
    add_shares(y3,tmp,c);

    mult_shares(tmp,b,y3);
    add_shares(y0,tmp,d);

    mult_shares(tmp,y3,y0);
    add_shares(y1,tmp,a);

    mult_shares(tmp,y0,y1);
    add_shares(c,tmp,b);

    local_memcpy(a,y0,D);
    local_memcpy(b,y1,D);
    local_memcpy(d,y3,D);
}

void lbox_inv_masked(clyde128_masked_state masked_state){
    for(uint32_t i=0;i<D;i++){
        lbox_inv_protected(&masked_state[0*D + i],&masked_state[1*D +i]);
        lbox_inv_protected(&masked_state[2*D + i],&masked_state[3*D +i]);
    }
}
