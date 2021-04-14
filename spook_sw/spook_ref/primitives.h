/* Spook Reference Implementation v1
 *
 * Written in 2019 at UCLouvain (Belgium) by Olivier Bronchain, Gaetan Cassiers
 * and Charles Momin.
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication along with
 * this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 */
#ifndef _H_PRIMITIVES_H_
#define _H_PRIMITIVES_H_

#include <stdint.h>
#include "parameters.h"

#define CLYDE128_NBYTES 16

#if SMALL_PERM
#define SHADOW_NBYTES 48
#else
#define SHADOW_NBYTES 64
#endif // SMALL_PERM

#define LS_ROWS 4      // Rows in the LS design
#define LS_ROW_BYTES 4 // number of bytes per row in the LS design
#define MLS_BUNDLES                                                            \
  (SHADOW_NBYTES / (LS_ROWS* LS_ROW_BYTES)) // Bundles in the mLS design

#define CLYDE_TYPE "clyde_32bit.c"
#define SHADOW_TYPE "shadow_128bit.c"

#define ROT32(x,n) ((uint32_t)(((x)>>(n))|((x)<<(32-(n)))))
typedef __attribute__((aligned(16))) uint32_t clyde128_state[LS_ROWS];
typedef __attribute__((aligned(64))) clyde128_state shadow_state[MLS_BUNDLES];

void clyde128_encrypt(clyde128_state state,
                      const clyde128_state t, uint32_t* k);

#ifdef INVERSE
void clyde128_decrypt(clyde128_state state,
                      const clyde128_state t, uint32_t* k);
static void lbox_inv(uint32_t* x, uint32_t* y);
#endif
void shadow(shadow_state state);

#endif //_H_PRIMITIVES_H_
