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
int crypto_aead_encrypt(unsigned char* c, uint32_t * clen,
                        const unsigned char* m, uint32_t  mlen,
                        const unsigned char* ad, uint32_t adlen,
                        const unsigned char* nsec, const unsigned char* npub,
                        uint32_t* k);

int crypto_aead_decrypt(unsigned char* m, uint32_t * mlen,
                        unsigned char* nsec, const unsigned char* c,
                        uint32_t clen, const unsigned char* ad,
                        uint32_t adlen, const unsigned char* npub,
                        uint32_t* k);
