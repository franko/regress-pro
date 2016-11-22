#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <mhash.h>
#include <gmp.h>

#include "registration.h"

#define BITSTRENGTH  256               /* size of modulus (n) in bits */
#define PRIMESIZE    (BITSTRENGTH / 2)  /* size of the primes p and q  */

#define SIGNBASE 62
#define SIGNBITCHAR 6

#define KEYBASE 16

/* size of a char buffer to hold the signature plus the terminating zero */
#define SIGNSIZE 172

#ifdef WIN32
static char * stpcpy(char *dst, const char *src)
{
    for(; *src; dst++, src++) {
        *dst = *src;
    }
    *dst = 0;
    return dst;
}
#endif

/* public key */
const char * const e_str = "10001";
const char * const n_str = "820e35bbf1b0b429de97078e4d56839f09438ad05faaa2bec1e7a278ec86ed45";

static bool
md5(mpz_t res, const char *msg)
{
    MHASH td;
    char *hash;
    int i;

    td = mhash_init(MHASH_MD5);

    if(td == MHASH_FAILED) {
        return false;
    }

    mhash(td, msg, strlen(msg));

    hash = mhash_end(td);

    int hash_size = mhash_get_block_size(MHASH_MD5);

    mpz_t pw;
    mpz_init_set_ui(pw, 1);

    mpz_set_ui(res, 0);
    for(i = 0; i < hash_size; i++) {
        unsigned char c = hash[i];
        mpz_addmul_ui(res, pw, c);
        mpz_mul_ui(pw, pw, 256);
    }

    mpz_clear(pw);
    free(hash);

    return true;
}

bool
registration_check(const char *user_name, const char *user_email,
                   int product_version, const char *signature)
{
    int cont_len = strlen(user_name) + strlen(user_email) + 32;
    char *reg_msg = malloc(cont_len * sizeof(char));
    char vbuf[32];
    char *ptr = reg_msg;
    bool verif;

    if(! reg_msg) {
        fprintf(stderr, "cannot allocate memory\n");
        return false;
    }

    mpz_t e, n;
    mpz_init_set_str(e, e_str, KEYBASE);
    mpz_init_set_str(n, n_str, KEYBASE);

    ptr = stpcpy(ptr, user_name);
    ptr = stpcpy(ptr, "\n");

    ptr = stpcpy(ptr, user_email);
    ptr = stpcpy(ptr, "\n");

    sprintf(vbuf, "%03i", product_version);
    ptr = stpcpy(ptr, vbuf);
    // no newline at the end

    mpz_t h, c, M;
    mpz_init(h);
    mpz_init(c);

    assert(md5(h, reg_msg));

    mpz_init_set_str(M, signature, SIGNBASE);

    /* c = M^e(mod n) */
    mpz_powm(c, M, e, n);

    verif = (mpz_cmp(c, h) == 0);

    mpz_clear(M);
    mpz_clear(c);
    mpz_clear(h);

    mpz_clear(e);
    mpz_clear(n);

    free(reg_msg);

    return verif;
}
