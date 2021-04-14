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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "prng.h"
#include "primitives.h"
#include "parameters.h"
#include "utils_masking.h"
#include "clyde_masked.h"

uint32_t unmask(const uint32_t *shares){
    uint32_t acc = 0;
    uint32_t i;
    for(i = 0;i<D;i++){
        acc ^= *shares;
        shares++;
    }
    return acc;
}

uint32_t refresh(uint32_t *shares){
    #if D<4
	refresh_block_j(shares,1);
    #elif D<9
        refresh_block_j(shares,1);
        refresh_block_j(shares,3);
    #else
	#error "Refresh not implemention for more than 8 shares"
    #endif
}



//////////////////////////////////
/////// This funnction is also implemented in Assembly
/////// These are only compile is USE_ASM is not set
//////////////////////////////////
#ifndef USE_ASM
uint32_t refresh_block_j(uint32_t *shares,uint32_t j){
    for(uint32_t i=0;i<D;i++){
        uint32_t r = get_random();
        shares[i] ^= r;
        shares[(i+j)%D] ^= r;
    }
}
// Apply a L-box to a pair of Clyde-128 rows.
static void lbox(uint32_t* x, uint32_t* y) {
  uint32_t a, b, c, d;
  a = *x ^ ROT32(*x, 12);
  b = *y ^ ROT32(*y, 12);
  a = a ^ ROT32(a, 3);
  b = b ^ ROT32(b, 3);
  a = a ^ ROT32(*x, 17);
  b = b ^ ROT32(*y, 17);
  c = a ^ ROT32(a, 31);
  d = b ^ ROT32(b, 31);
  a = a ^ ROT32(d, 26);
  b = b ^ ROT32(c, 25);
  a = a ^ ROT32(c, 15);
  b = b ^ ROT32(d, 15);
  *x = a;
  *y = b;
}
static void lbox_inv(uint32_t* x, uint32_t* y) {
  uint32_t a, b, c, d;
  a = *x ^ ROT32(*x, 25);
  b = *y ^ ROT32(*y, 25);
  c = *x ^ ROT32(a, 31);
  d = *y ^ ROT32(b, 31);
  c = c ^ ROT32(a, 20);
  d = d ^ ROT32(b, 20);
  a = c ^ ROT32(c, 31);
  b = d ^ ROT32(d, 31);
  c = c ^ ROT32(b, 26);
  d = d ^ ROT32(a, 25);
  a = a ^ ROT32(c, 17);
  b = b ^ ROT32(d, 17);
  a = ROT32(a, 16);
  b = ROT32(b, 16);
  *x = a;
  *y = b;
}
uint32_t lbox_inv_protected(uint32_t *x, uint32_t *y){
    lbox_inv(x,y);
}
uint32_t lbox_protected(uint32_t *x, uint32_t *y){
    lbox(x,y);
}
void XORLS_MASK(clyde128_masked_state DEST, uint32_t *OP){
    DEST[0] ^=OP[0];
    DEST[1*D] ^=OP[1];
    DEST[2*D] ^=OP[2];
    DEST[3*D] ^=OP[3];
}

void XORCST_MASK( clyde128_masked_state DEST, uint32_t LFSR){
    DEST[0] ^= (LFSR>>3) & 0x1;
    DEST[D] ^= (LFSR>>2) & 0x1;
    DEST[2*D] ^= (LFSR>>1) & 0x1;
    DEST[3*D] ^= (LFSR>>0) & 0x1;
}

void add_shares(uint32_t *out,const uint32_t *a, const uint32_t *b){
    int32_t i;
    for(i=(D-1);i>=0;i--){
        out[i] = a[i] ^ b[i];
    }
    return;
}

void local_memcpy(uint32_t *out, const uint32_t *in, uint32_t len){
    for(uint32_t i=0;i<len;i++)
        out[i] = in[i];
}

void local_memcpy_bytes(void *out, const void *in, uint32_t len){
    uint8_t *out_c = (uint8_t *) out;
    uint8_t *in_c = (uint8_t *) in;
    for(uint32_t i=0;i<len;i++)
        out_c[i] = in_c[i];
}

void add_clyde128_masked_state(clyde128_masked_state out,const clyde128_masked_state a,const clyde128_masked_state b){
    for(int d=(D-1);d>=0;d--){
        for(int i=0;i<4;i++){
            out[(i*D) + d] = a[(i*D)+d] ^ b[(i*D)+d];
        }
    }
}

void mult_shares(uint32_t *out,const uint32_t *a, const uint32_t *b){
    uint32_t i,j;
    uint32_t s,sp,tmp;
    uint32_t tmpa,tmpb;
    for(i=0;i<D;i++){
        tmpa = a[i];
        tmpb = b[i];
        out[i] = tmpa & tmpb;
    }
    for(i=0;i<D;i++){
        for(j=i+1;j<D;j++){
            s = get_random();
            tmp = (a[i] & b[j]) ^ s;
            sp = tmp ^ (a[j] & b[i]);
            out[i] ^= s;
            out[j] ^= sp;
        }
    }
}
#endif // USE_ASM
