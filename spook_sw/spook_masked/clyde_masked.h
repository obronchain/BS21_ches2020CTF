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

#ifndef __CLYDE_MASKED__
#define __CLYDE_MASKED__
#ifndef D
    #define D 1
#endif // D

#define DM (D-1)

#ifndef ASM
#include <stdint.h>
typedef __attribute__((aligned(16))) uint32_t clyde128_state[4];

/*
 * the 4 x 32-bit of state of clyde shared states. shares of the same
 * 32-bit of state are layed out consecutively in memory.
 */
typedef __attribute__((aligned(16))) uint32_t clyde128_masked_state[4*D];

/* func: sbox_layer
 * desc: applies an sbox layer to the inputs shares.
 *
 * inputs: the d shares of a variable (i.e. a) are stored in a uint32_t[d].
 *
 * outputs: inputs are updated with shares of the output of the sbox
 */
void sbox_layer_masked(uint32_t* a,uint32_t *b, uint32_t *c, uint32_t *d,uint32_t refresh);
void sbox_inv_layer_masked(uint32_t* a,uint32_t *b, uint32_t *c, uint32_t *d);

/* func: lbox_layer
 * desc: applies an lbox layer to the inputs shared state
 *
 * inputs: a clyde128_masked_state
 *
 * outputs: inputs are updated with shares of the output of the lbox
 *
 * note: since it is a linear operation, it is performed on
 * each shares independently.
*/
void lbox_masked(clyde128_masked_state masked_state);
void lbox_inv_masked(clyde128_masked_state masked_state);

/* func: add_clyde128_masked_state
 * desc: perform sharewise addition betweeb a and b. Store the result in out
 */
void add_clyde128_masked_state(clyde128_masked_state out,const clyde128_masked_state a,const clyde128_masked_state b);
#endif // ASM

#endif // __CLYDE_MASKED__
