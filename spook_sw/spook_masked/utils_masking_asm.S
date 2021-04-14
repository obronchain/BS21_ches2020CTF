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

#define ASM
#include "compiler_abstraction.h"
#include "clyde_masked.h"

CODESECT
    EXPORT local_memcpy
    EXPORT local_memcpy_bytes
    EXPORT XORLS_MASK
    EXPORT XORCST_MASK
    EXPORT add_shares

    EXPORT add_clyde128_masked_state
    EXPORT refresh_block_j
    EXPORT mult_shares
    EXPORT lbox_protected
    EXPORT lbox_inv_protected
//////////////////////////////////////////
//////////////////////////////////////////
// File utils_masking.c contains C version
// of all the functions below
//////////////////////////////////////////
//////////////////////////////////////////
/*******************************************
#    @func: local_memcpy
#    @desc: copy the uint32_t pointer data to another
#    @inputs:
#        r0 -> *r_out
#        r1 -> *r_in
#        r2 -> len
********************************************/
local_memcpy:
    cmp r2,#0
    bcc end_memcpy
    lsls r2,#2  // four bytes offset, points to last index
    movs r3,#0  // clean
    loop_memcpy:
        adds r2,r2,#-4
        str r2,[r0,r2] // dump
        ldr r2,[r0,r2] // dump
        ldr r3,[r1,r2]
        str r3,[r0,r2]
        movs r3,#0 // clean reg
        cmp r2,#0
        bne loop_memcpy

    end_memcpy:
        bx lr
/*******************************************
#    @func: local_memcpy_bytes
#    @desc: copy the uint8_t pointer data to another
#    @inputs:
#        r0 -> *r_out
#        r1 -> *r_in
#        r2 -> len
********************************************/
local_memcpy_bytes:
    cmp r2,#0
    bcc end_memcpy_b
    movs r3,#0
    loop_memcpy_b:
        adds r2,r2,#-1
        ldrb r3,[r1,r2]
        strb r3,[r0,r2]
        movs r3,#0
        cmp r2,#0
        bne loop_memcpy_b

    end_memcpy_b:
        bx lr
/*******************************************
#    @func: add_shares
#    @desc: add two D-shares vectors
#    @inputs:
#        r0 -> *r_out
#        r1 -> *a
#        r2 -> *b
********************************************/
add_shares:
    push {r4-r5}
    movs r3,D   // loop counter
    lsls r3,#2
    // clear r4 and r5
    movs r4,#0
    movs r5,#0

    loop_add_shares:
        adds r3,r3,#-4

        // load shares i
        ldr r4,[r1,r3]
        ldr r5,[r2,r3]
        // xor and store the shares
        eors r4,r4,r5
        str r4,[r0,r3]

        // clear r4 and r5
        movs r4,#0
        movs r5,#0

        // loop check
        cmp r3,#0
        bne loop_add_shares
    pop {r4-r5}
    bx lr

/*******************************************
#    @func: XORLS_MASK
#    @desc: XOR the first share of the state (*r0) with *(r1)
#    @inputs:
#        r0 -> DEST
#        r1 -> OP
********************************************/
XORLS_MASK:
    // D0
    push {r4}

    ldr r2,[r0,#0]
    ldr r3,[r1,#0]
    eors r2,r2,r3
    str r2,[r0,#0]

    movs r4, D*4
    ldr r2,[r0,r4]
    ldr r3,[r1,#4]
    eors r2,r2,r3
    str r2,[r0,r4]

    movs r4, D*8
    ldr r2,[r0,r4]
    ldr r3,[r1,#8]
    eors r2,r2,r3
    str r2,[r0,r4]

    movs r4, D*12
    ldr r2,[r0,r4]
    ldr r3,[r1,#12]
    eors r2,r2,r3
    str r2,[r0,r4]

    movs r2,#0
    movs r3,#0
    pop {r4}
    bx lr

/*******************************************
#    @func: XORLS_MASK
#    @desc: Performs the addition on contant between the
#       state (DEST) and the LFSR.
#    @inputs:
#        r0 -> DEST
#        r1 -> LFSR
********************************************/
XORCST_MASK:
    push {r4-r5}
    movs r5,0x1

    movs r3,D*12
    ldr r2,[r0,r3]
    movs r4,r1
    ands r4,r4,r5
    eors r2,r2,r4
    str r2,[r0,r3]
    lsrs r1,#1

    movs r3,D*8
    ldr r2,[r0,r3]
    movs r4,r1
    ands r4,r4,r5
    eors r2,r2,r4
    str r2,[r0,r3]
    lsrs r1,#1

    movs r3,D*4
    ldr r2,[r0,r3]
    movs r4,r1
    ands r4,r4,r5
    eors r2,r2,r4
    str r2,[r0,r3]
    lsrs r1,#1

    movs r3,D*0
    ldr r2,[r0,r3]
    movs r4,r1
    ands r4,r4,r5
    eors r2,r2,r4
    str r2,[r0,r3]
    movs r2,#0
    movs r3,#0
    pop {r4-r5}
    bx lr
/*******************************************
#    @func: refresh_block_j
#    @desc: refresh shares with rotation of j
#    @inputs:
#        r0 -> *shares
#        r1 -> j
#
#    @regs alloc:
#       r0 -> rng
#       r4 -> cnt i
#       r5 -> shares
#       r6 -> j
#
********************************************/
refresh_block_j:
    push {r4-r7}
    movs r4,#0   // loop counter
    movs r5,r0  // shares addr
    movs r6,r1  // j
    movs r7,#0  // subs
    loop_rbj:
        // get_rng
        push {lr}
        bl get_random
        pop {r1}
        mov lr,r1

        // load shares i
        lsls r3,r4,#2
        ldr r1,[r5,r3]
        eors r1,r1,r0
        str r1,[r5,r3]
        movs r1,#0

        // load (i+j)%D
        lsrs r3,r3,#2 // i
        adds r3,r3,r6 // i+j
        cmp r3,D
        bne cnt_block
        movs r7,D
    cnt_block:
        subs r3,r3,r7 // (i+j)%D
        lsls r3,#2

        // load shares (i+j)%D
        ldr r1,[r5,r3]
        eors r1,r1,r0
        str r1,[r5,r3]
        movs r1,#0
        movs r0,#0

        // loop check
        adds r4,r4,#1
        cmp r4,D
        bne loop_rbj

    pop {r4-r7}
    bx lr

/*******************************************
#    @func: lbox_inv_protected
#    @desc: lbox_inv and then clean regs
#    @inputs:
#        r0 -> *x
#        r1 -> *y
#        r2 -> a
#        r3 -> b
#        r4 -> c
#        r5 -> d
#        r6 -> offset
#        r7 -> tmp
#   This has been generating from C code with gcc
#   We only added register cleaning at the end
********************************************/
#ifdef INVERSE
lbox_inv_protected:
	ldr	r3, [r0, #0]
	push	{r4, r5, r6, r7, lr}
	movs	r7, #25
	movs	r5, r3
	ldr	r6, [r1, #0]
	rors	r5, r7
	rors	r6, r7
	ldr	r2, [r1, #0]
	eors	r5, r3
	eors	r6, r2
	movs	r4, r5
	movs	r2, #31
	rors	r4, r2
	mov	lr, r8
	mov	r8, r4
	movs	r4, #20
	mov	ip, r0
	rors	r5, r4
	mov	r0, r8
	eors	r5, r0
	eors	r5, r3
	movs	r3, r6
	rors	r6, r4
	rors	r3, r2
	ldr	r4, [r1, #0]
	eors	r3, r6
	eors	r4, r3
	movs	r6, r4
	rors	r6, r2
	movs	r3, r5
	eors	r6, r4
	movs	r0, r6
	rors	r3, r2
	subs	r2, #5
	rors	r0, r2
	movs	r2, r0
	eors	r3, r5
	eors	r2, r5
	movs	r5, #17
	rors	r2, r5
	eors	r2, r3
	rors	r3, r7
	eors	r3, r4
	rors	r3, r5
	movs	r4, #16
	eors	r3, r6
	rors	r2, r4
	mov	r0, ip
	rors	r3, r4
	push	{lr}
	str	r2, [r0, #0]
	str	r3, [r1, #0]
        movs    r0,#0
        movs    r1,#0
        movs    r2,#0
        movs    r3,#0
        movs    r4,#0
        movs    r4,#0
        movs    r6,#0
        movs    r7,#0
        movs    r7,#0
	mov 	r8,r7
	pop	{r2}
	mov	r8, r2
	pop	{r4, r5, r6, r7, pc}
#endif
/*******************************************
#    @func: lbox_protected
#    @desc: lbox and then clean regs
#    @inputs:
#        r0 -> *x
#        r1 -> *y
#        r2 -> a
#        r3 -> b
#        r4 -> c
#        r5 -> d
#        r6 -> offset
#        r7 -> tmp
#   This has been generating from C code with gcc
#   We only added register cleaning at the end
********************************************/
lbox_protected:
    push	{r4, r5, r6, r7, lr}
    ldr	r4, [r0, #0]
    movs	r3, #12
    movs	r7, r4
    ldr	r6, [r1, #0]
    rors	r7, r3
    rors	r6, r3
    ldr	r3, [r1, #0]
    eors	r7, r4
    eors	r6, r3
    movs	r2, r7
    movs	r3, #3
    movs	r5, r6
    rors	r2, r3
    rors	r5, r3
    adds	r3, #14
    rors	r4, r3
    eors	r4, r7
    eors	r4, r2
    ldr	r2, [r1, #0]
    rors	r2, r3
    eors	r2, r6
    eors	r5, r2
    movs	r6, r5
    movs	r2, #31
    movs	r3, r4
    rors	r6, r2
    rors	r3, r2
    movs	r2, r6
    eors	r2, r5
    movs	r6, #26
    movs	r7, r2
    rors	r7, r6
    movs	r6, r7
    eors	r3, r4
    eors	r6, r4
    movs	r4, #15
    rors	r2, r4
    eors	r2, r5
    movs	r5, r3
    rors	r5, r4
    movs	r4, r5
    eors	r4, r6
    str	r4, [r0, #0]
    movs	r0, #25
    rors	r3, r0
    eors	r2, r3
    str	r2, [r1, #0]
    movs    r0,#0
    movs    r1,#0
    movs    r2,#0
    movs    r3,#0
    movs    r4,#0
    movs    r4,#0
    movs    r6,#0
    movs    r7,#0
    pop	{r4, r5, r6, r7, pc}

/*******************************************
#    @func: add_clyde128_masked_state
#    @desc: add two D-shares of state
#    @inputs:
#        r0 -> *r_out
#        r1 -> *a
#        r2 -> *b
********************************************/
add_clyde128_masked_state:
    push {r4-r7}
    movs r3,D
    add_state_loop1:
        adds r3,r3,#-1

        // i = 0, offset = (0*D)+r3
        lsls r7,r3,#2
        ldr r5,[r1,r7]
        ldr r6,[r2,r7]
        eors r5,r5,r6
        str r5,[r0,r7]
        movs r6,#0
        movs r5,#0

        // i = 1, offset = (1*D)+r3
        movs r4,r3
        adds r4,r4,D
        lsls r7,r4,#2
        ldr r5,[r1,r7]
        ldr r6,[r2,r7]
        eors r5,r5,r6
        str r5,[r0,r7]
        movs r6,#0
        movs r5,#0

        // i = 2, offset = (2*D)+r3
        adds r4,r4,D
        lsls r7,r4,#2
        ldr r5,[r1,r7]
        ldr r6,[r2,r7]
        eors r5,r5,r6
        str r5,[r0,r7]
        movs r6,#0
        movs r5,#0

        // i = 3, offset = (3*D)+r3
        adds r4,r4,D
        lsls r7,r4,#2
        ldr r5,[r1,r7]
        ldr r6,[r2,r7]
        eors r5,r5,r6
        str r5,[r0,r7]
        movs r6,#0
        movs r5,#0

        cmp r3,#0
        bne add_state_loop1

    pop {r4-r7}
    bx lr

//////////////////////////////////////////
///// AND GATE
//////////////////////////////////////////
/*******************************************
#    @func: mult_shares
#    @desc: mult two D-shares vectors with a plain ISW
#    @inputs:
#        r0 -> *r_out
#        r1 -> *a
#        r2 -> *b
#    @reg allocation:
#       r0 -> s
#       r1 -> tmp
#       r2 -> tmp
#       r3 -> tmp
#       r4 -> a[i]          -> preserved
#       r5 -> b[i]          -> preserved
#       r6 -> cnt loop1     -> preserved
#       r7 -> cnt loop2     -> preserved
#       r8 -> &out          -> preserved
#       r9 -> tmp           -> not used
#       r10 -> &a           -> preserved
#       r11 -> &b           -> preservedved
********************************************/
mult_shares:
    push {r4-r7,lr}
    mov r3,r8
    mov r4,r9
    mov r5,r10
    mov r6,r11
    push {r3-r6}
    mov r8,r0 // &out
    mov r10,r1 // &a
    mov r11,r2 // &b

    movs r3,#0 // this is the counter
    mov r9,r3
    movs r6,#0
    movs r4,#0
    loop_ISW_0:
        // load shares i
        lsls r3,#2
        ldr r4,[r1,r3]
        ldr r6,[r2,r3]
        // and and store the shares
        ands r4,r4,r6
        str r4,[r0,r3]
        movs r4,#0      // clean reg
        movs r6,#0      // clean reg

        lsrs r3,#2
        adds r3,r3,#1
        cmp r3,D
        bne loop_ISW_0
    movs r6,#0 // counter loop 1
    loop_ISW_1:
        // load shares i
        adds r7,r6,#1        // counter loop 2
        lsls r6,r6,#2
        loop_ISW_2:
            // load other shares
            lsls r7,r7,#2
            bl get_random
            movs r3,#0
            mov r9,r3
            movs r2,#0
            movs r1,#0

            //////////////////////////////////////////////
            /// computing out[i] ^=s
            //////////////////////////////////////////////
            mov r2,r8      // access to out[i]
            ldr r2,[r2,r6] // r2 = out[i]
            eors r2,r2,r0  // r2 = out[i] ^=s
            mov r1,r8      // access to out[i]
            str r2,[r1,r6] // out[i] ^= s
            movs r2,#0     // clear r2

            /////////////////////////////////////////
            /// computing a[i] & b[j]
            /////////////////////////////////////////
            ///
            mov r4,r10      // r1 = a
            ldr r4,[r4,r6]  // a[i]
            ///
            mov r3,r11
            ldr r3,[r3,r7]  // b[j]
            ands r3,r3,r4   // a[i] & b[j]
            eors r3,r3,r0   // tmp = (a[i] & b[j]) ^ s
            movs r4,#0
            movs r0,#0
            //////////////////////////////////////////
            /// computing a[j] & b[i]
            //////////////////////////////////////////

            ///
            mov r4,r10
            ldr r4,[r4,r7]      // a[j]
            ///
            mov r1,r11
            ldr r1,[r1,r6]  // b[i]
            ands r1,r1,r4   // b[i] & a[j]
            movs r4,#0
            eors r3,r3,r1   // sp = tmp ^ (b[i]&a[j])
            movs r1,#0

            ////////////////////////////////////////////////
            ////// computing out[j] ^= sp
            ////////////////////////////////////////////////
            ///
            // access out[j]
            mov r2,r8
            ldr r2,[r2,r7]
            mov r1,r8
            eors r2,r2,r3
            movs r3,#0
            str r2,[r1,r7]  // out[j] ^=sp
            movs r2,#0
            lsrs r7,r7,#2
            adds r7,r7,#1
            movs r2,#0
            cmp r7,D
            bne loop_ISW_2

        lsrs r6,#2
        adds r6,r6,#1
        cmp r6,DM
        bne loop_ISW_1

    pop {r3-r6}
    mov r11,r6
    mov r10,r5
    mov r9,r4
    mov r8,r3
    pop {r4-r7,pc}
