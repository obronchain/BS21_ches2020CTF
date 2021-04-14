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

/* func: get_random
 * This function return a 32-bit random value out of the prng
 */
uint32_t get_random();

/* func: init_rng
 * seed is a table of size 4 of uint32_t. This is used to initialize
 * the prng state. It is based on Shadow permutation in a spong mode.
 */
void init_rng(uint32_t *seed);


/* func: fill_table
 * When called, this will fill a table with random value. This is i.e. used
 * to recompute randomness before the measurements.
 */
void fill_table();
