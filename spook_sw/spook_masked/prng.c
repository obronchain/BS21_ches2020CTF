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
#include <string.h>
#include <time.h>
#include <stdint.h>
#include "primitives.h"
#include "parameters.h"
#include "utils_masking.h"
#include "clyde_masked.h"
#define MAX 256
static uint32_t prng_tab[MAX];
static uint32_t prng_index;
static shadow_state prng_state_core;
#define PRGON 1
#if PRGON==1

void fill_table(){
    for(int i=0;i<MAX;i+=8){
        shadow(prng_state_core);
        memcpy(&prng_tab[i],prng_state_core,32);
    }
    prng_index = 0;
}
void init_rng(uint32_t *seed){
    memset(prng_state_core,0,64);
    memcpy(prng_state_core[0],seed,16);
    prng_index = MAX;
}

uint32_t get_random(){
    if(prng_index >= MAX){
        fill_table();
    }
    uint32_t r = prng_tab[prng_index];
    prng_index +=1 ;

    return r;
}

#else
void fill_table(){
    return;
}
void init_rng(uint32_t *seed){
    return;
}

uint32_t get_random(){
    return 0;
}

#endif
