/* ---------------------------------------------------------------------------------------------------------------------
   THIS IS A MODIFICATION OF AN IMPLEMENTATION PROVIDED BY TAKUJI NISHIMRU AND MAKOTO MATSUMOTO.
   THEIR ORIGINAL LICENSE IS PROVIDED BELOW.
------------------------------------------------------------------------------------------------------------------------
   A C-program for MT19937-64 (2014/2/23 version).
   Coded by Takuji Nishimura and Makoto Matsumoto.

   This is a 64-bit version of Mersenne Twister pseudorandom number
   generator.

   Before using, initialize the state by using init_genrand64(seed)  
   or init_by_array64(init_key, key_length).

   Copyright (C) 2004, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   References:
   T. Nishimura, ``Tables of 64-bit Mersenne Twisters''
     ACM Transactions on Modeling and 
     Computer Simulation 10. (2000) 348--357.
   M. Matsumoto and T. Nishimura,
     ``Mersenne Twister: a 623-dimensionally equidistributed
       uniform pseudorandom number generator''
     ACM Transactions on Modeling and 
     Computer Simulation 8. (Jan. 1998) 3--30.

   Any feedback is very welcome.
   http://www.math.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove spaces)
--------------------------------------------------------------------------------------------------------------------- */

#include "../include/mt19937-64.h"

#include <time.h>

#define NN       HH_MT19937_NN
#define MM       156
#define MATRIX_A UINT64_C(0xB5026F5AA96619E9)
#define UM       UINT64_C(0xFFFFFFFF80000000)
#define LM       UINT64_C(0x7FFFFFFF)
#define LB                                   \
        {                                    \
                .mti = NN + 1, .init = false \
        }

hh_mt19937_ro_t hh_mt19937_global = LB;

hh_mt19937_ro_t hh_init_mt(void)
{
        hh_mt19937_ro_t out = LB;
        return out;
}

void hh_mt_init_basic(hh_mt19937_ro_t *o, bool pi_check)
{
        if (pi_check && o->init) return;

        hh_mt_init_genrand64(o, time(NULL));
}

void hh_mt_init_genrand64(hh_mt19937_ro_t *o, uint64_t seed)
{
        o->mt[0] = seed;
        for (o->mti = 1; o->mti < NN; o->mti++)
                o->mt[o->mti]
                    = (UINT64_C(6364136223846793005) * (o->mt[o->mti - 1] ^ (o->mt[o->mti - 1] >> 62)) + o->mti);

        o->init = true;
}

void hh_mt_init_by_array64(hh_mt19937_ro_t *o, uint64_t init_key[], uint64_t key_length)
{
        unsigned int i, j;
        uint64_t     k;
        hh_mt_init_genrand64(o, UINT64_C(19650218));
        i = 1;
        j = 0;
        k = (NN > key_length ? NN : key_length);
        for (; k; k--) {
                o->mt[i] = (o->mt[i] ^ ((o->mt[i - 1] ^ (o->mt[i - 1] >> 62)) * UINT64_C(3935559000370003845)))
                           + init_key[j] + j;
                i++;
                j++;
                if (i >= NN) {
                        o->mt[0] = o->mt[NN - 1];
                        i        = 1;
                }
                if (j >= key_length) j = 0;
        }
        for (k = NN - 1; k; k--) {
                o->mt[i] = (o->mt[i] ^ ((o->mt[i - 1] ^ (o->mt[i - 1] >> 62)) * UINT64_C(2862933555777941757))) - i;
                i++;
                if (i >= NN) {
                        o->mt[0] = o->mt[NN - 1];
                        i        = 1;
                }
        }

        o->mt[0] = UINT64_C(1) << 63;

        o->init = true;
}

uint64_t hh_mt_genrand64_int64(hh_mt19937_ro_t *o)
{
        int             i;
        uint64_t        x;
        static uint64_t mag01[2] = {UINT64_C(0), MATRIX_A};

        if (o->mti >= NN) {
                if (o->mti == NN + 1) hh_mt_init_genrand64(o, UINT64_C(5489));

                for (i = 0; i < NN - MM; i++) {
                        x        = (o->mt[i] & UM) | (o->mt[i + 1] & LM);
                        o->mt[i] = o->mt[i + MM] ^ (x >> 1) ^ mag01[(int)(x & UINT64_C(1))];
                }
                for (; i < NN - 1; i++) {
                        x        = (o->mt[i] & UM) | (o->mt[i + 1] & LM);
                        o->mt[i] = o->mt[i + (MM - NN)] ^ (x >> 1) ^ mag01[(int)(x & UINT64_C(1))];
                }
                x             = (o->mt[NN - 1] & UM) | (o->mt[0] & LM);
                o->mt[NN - 1] = o->mt[MM - 1] ^ (x >> 1) ^ mag01[(int)(x & UINT64_C(1))];

                o->mti = 0;
        }

        x = o->mt[o->mti++];

        x ^= (x >> 29) & UINT64_C(0x5555555555555555);
        x ^= (x << 17) & UINT64_C(0x71D67FFFEDA60000);
        x ^= (x << 37) & UINT64_C(0xFFF7EEE000000000);
        x ^= (x >> 43);

        return x;
}
