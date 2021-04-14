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

#include "primitives.h"
static void sbox_layer(uint32_t* state);
static void dbox_mls_layer(shadow_state state,uint32_t *lfsr);

#ifdef SHADOW
static uint32_t lfsr_poly;
static uint32_t xtime_poly;
#endif

// Apply a S-box layer to a Clyde-128 state.
static void sbox_layer(uint32_t* state) {
  uint32_t y1 = (state[0] & state[1]) ^ state[2];
  uint32_t y0 = (state[3] & state[0]) ^ state[1];
  uint32_t y3 = (y1 & state[3]) ^ state[0];
  uint32_t y2 = (y0 & y1) ^ state[3];
  state[0] = y0;
  state[1] = y1;
  state[2] = y2;
  state[3] = y3;
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
#ifdef SHADOW
void set_poly_lfsr(uint32_t l){
    lfsr_poly = l;
}
void set_poly_xtime(uint32_t l){
    xtime_poly = l;
}

static uint32_t update_lfsr(uint32_t x) {
    int32_t tmp1 = x;
    uint32_t tmp =  (tmp1 >>31) & lfsr_poly;
    return (x<<1) ^ tmp;
}

static uint32_t xtime(uint32_t x) {
    int32_t tmp1 = x;
    uint32_t tmp =  (tmp1 >>31) & xtime_poly;
    return (x<<1) ^ tmp;
}

// Apply a D-box layer to a Shadow state.
static void dbox_mls_layer(shadow_state state,uint32_t *lfsr) {
for (unsigned int row = 0; row < LS_ROWS; row++) {
#if SMALL_PERM
    uint32_t x1 = state[0][row];
    uint32_t x2 = state[1][row];
    uint32_t x3 = state[2][row];

    uint32_t a = x1 ^ x3;
    uint32_t b = a ^ x2;
    uint32_t c = xtime(a) ^ (x1 ^ x2);
    state[0][row] = a ^ c;
    state[1][row] = b;
    state[2][row] = c;

    state[0][row] ^= *lfsr;
    *lfsr = update_lfsr(*lfsr);

#else

    state[0][row] ^= state[1][row];
    state[2][row] ^= state[3][row];
    state[1][row] ^= state[2][row];
    state[3][row] ^= xtime(state[0][row]);
    state[2][row] ^= xtime(state[3][row]);
    state[1][row] = xtime(state[1][row]);
    state[0][row] ^= state[1][row];
    state[3][row] ^= state[0][row];
    state[1][row] ^= state[2][row];

    state[0][row] ^= *lfsr;
    *lfsr = update_lfsr(*lfsr);
#endif // SMALL_PERM
  }

}
#endif
