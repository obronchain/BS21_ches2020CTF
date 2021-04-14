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
#include <clyde_masked.h>
#define ROT32(x,n) ((uint32_t)(((x)>>(n))|((x)<<(32-(n)))))
/* func: umask
 * Unshare a sharing of 128-bit state.
 */
uint32_t unmask(const uint32_t *share);
/* func: lbox_protected
 * apply lbox to the shares of x and y.
 */
uint32_t lbox_protected(uint32_t *x, uint32_t *y);
/* func;: lbox_inv_protected
 * apply lbox_inv to the shares of x and y.
 */
uint32_t lbox_inv_protected(uint32_t *x, uint32_t *y);
/* func: add_shares
 * performs addition between the sharing in a and b and store
 * the result in out
 */
void add_shares(uint32_t *out,const uint32_t *a, const uint32_t *b);
/* func: mult_shares
 * performs multiplication between the sharing in a and b and store
 * the result in out. This is down with an ISW multiplication.
 */
void mult_shares(uint32_t *out,const uint32_t *a, const uint32_t *b);
/* func: local_memcpy
 * Copy one uint32_t pointer to another one. Compared to memcpy,
 * it cleans registers when implemented in assembly.
 */
void local_memcpy(uint32_t *dest, const uint32_t *orig, uint32_t len);
void local_memcpy_bytes(void *out, const void *in, uint32_t len);
/* func: refresh
 * Performs an in place SNI refresh on the sharing.
 * This refresh is implemented up to D=8.
 */
uint32_t refresh(uint32_t *shares);
/* func: refresh_block_j
 * Performs addition of a shares of zeros where the
 * randomness is shifted by j.
 * output: shares ^ r ^ r<<j
 */
uint32_t refresh_block_j(uint32_t *shares,uint32_t j);
void XORLS_MASK( clyde128_masked_state DEST, uint32_t *OP);
void XORCST_MASK( clyde128_masked_state DEST, uint32_t LFSR);
