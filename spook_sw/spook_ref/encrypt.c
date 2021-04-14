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
#include "crypto_aead.h"
#include "s1p.h"
#include <stdint.h>
#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

// Spook encryption.
int crypto_aead_encrypt(unsigned char* c, uint32_t* clen,
                        const unsigned char* m, uint32_t mlen,
                        const unsigned char* ad, uint32_t adlen,
                        const unsigned char* nsec UNUSED,
                        const unsigned char* npub, uint32_t * k) {
  unsigned char p[P_NBYTES];
  uint32_t * k_priv;
  init_keys(&k_priv, p, k);
  s1p_encrypt(c, clen, ad, adlen, m, mlen, k_priv, p, npub);
  return 0;
}

// Spook encryption.
int crypto_aead_decrypt(unsigned char* m, uint32_t* mlen,
                        unsigned char* nsec UNUSED, const unsigned char* c,
                        uint32_t clen, const unsigned char* ad,
                        uint32_t adlen, const unsigned char* npub,
                        uint32_t * k) {
  unsigned char p[P_NBYTES];
  uint32_t* k_priv;
  init_keys(&k_priv, p, k);
  return s1p_decrypt(m, mlen, ad, adlen, c, clen, k_priv, p, npub);
}
