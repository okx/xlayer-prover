#include "fq.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <assert.h>
#include <string>


static mpz_t q;
static mpz_t zero;
static mpz_t one;
static mpz_t mask;
static size_t nBits;
static bool initialized = false;


void Fq_toMpz(mpz_t r, PFqElement pE) {
    FqElement tmp;
    Fq_toNormal(&tmp, pE);
    if (!(tmp.type & Fq_LONG)) {
        mpz_set_si(r, tmp.shortVal);
        if (tmp.shortVal<0) {
            mpz_add(r, r, q);
        }
    } else {
        mpz_import(r, Fq_N64, -1, 8, -1, 0, (const void *)tmp.longVal);
    }
}

void Fq_fromMpz(PFqElement pE, mpz_t v) {
    if (mpz_fits_sint_p(v)) {
        pE->type = Fq_SHORT;
        pE->shortVal = mpz_get_si(v);
    } else {
        pE->type = Fq_LONG;
        for (int i=0; i<Fq_N64; i++) pE->longVal[i] = 0;
        mpz_export((void *)(pE->longVal), NULL, -1, 8, -1, 0, v);
    }
}


bool Fq_init() {
    if (initialized) return false;
    initialized = true;
    mpz_init(q);
    mpz_import(q, Fq_N64, -1, 8, -1, 0, (const void *)Fq_q.longVal);
    mpz_init_set_ui(zero, 0);
    mpz_init_set_ui(one, 1);
    nBits = mpz_sizeinbase (q, 2);
    mpz_init(mask);
    mpz_mul_2exp(mask, one, nBits);
    mpz_sub(mask, mask, one);
    return true;
}

void Fq_str2element(PFqElement pE, char const *s) {
    mpz_t mr;
    mpz_init_set_str(mr, s, 10);
    mpz_fdiv_r(mr, mr, q);
    Fq_fromMpz(pE, mr);
    mpz_clear(mr);
}

char *Fq_element2str(PFqElement pE) {
    FqElement tmp;
    mpz_t r;
    if (!(pE->type & Fq_LONG)) {
        if (pE->shortVal>=0) {
            char *r = new char[32];
            sprintf(r, "%d", pE->shortVal);
            return r;
        } else {
            mpz_init_set_si(r, pE->shortVal);
            mpz_add(r, r, q);
        }
    } else {
        Fq_toNormal(&tmp, pE);
        mpz_init(r);
        mpz_import(r, Fq_N64, -1, 8, -1, 0, (const void *)tmp.longVal);
    }
    char *res = mpz_get_str (0, 10, r);
    mpz_clear(r);
    return res;
}

void Fq_idiv(PFqElement r, PFqElement a, PFqElement b) {
    mpz_t ma;
    mpz_t mb;
    mpz_t mr;
    mpz_init(ma);
    mpz_init(mb);
    mpz_init(mr);

    Fq_toMpz(ma, a);
    // char *s1 = mpz_get_str (0, 10, ma);
    // printf("s1 %s\n", s1);
    Fq_toMpz(mb, b);
    // char *s2 = mpz_get_str (0, 10, mb);
    // printf("s2 %s\n", s2);
    mpz_fdiv_q(mr, ma, mb);
    // char *sr = mpz_get_str (0, 10, mr);
    // printf("r %s\n", sr);
    Fq_fromMpz(r, mr);

    mpz_clear(ma);
    mpz_clear(mb);
    mpz_clear(mr);
}

void Fq_mod(PFqElement r, PFqElement a, PFqElement b) {
    mpz_t ma;
    mpz_t mb;
    mpz_t mr;
    mpz_init(ma);
    mpz_init(mb);
    mpz_init(mr);

    Fq_toMpz(ma, a);
    Fq_toMpz(mb, b);
    mpz_fdiv_r(mr, ma, mb);
    Fq_fromMpz(r, mr);

    mpz_clear(ma);
    mpz_clear(mb);
    mpz_clear(mr);
}

void Fq_pow(PFqElement r, PFqElement a, PFqElement b) {
    mpz_t ma;
    mpz_t mb;
    mpz_t mr;
    mpz_init(ma);
    mpz_init(mb);
    mpz_init(mr);

    Fq_toMpz(ma, a);
    Fq_toMpz(mb, b);
    mpz_powm(mr, ma, mb, q);
    Fq_fromMpz(r, mr);

    mpz_clear(ma);
    mpz_clear(mb);
    mpz_clear(mr);
}

void Fq_inv(PFqElement r, PFqElement a) {
    mpz_t ma;
    mpz_t mr;
    mpz_init(ma);
    mpz_init(mr);

    Fq_toMpz(ma, a);
    mpz_invert(mr, ma, q);
    Fq_fromMpz(r, mr);
    mpz_clear(ma);
    mpz_clear(mr);
}

void Fq_div(PFqElement r, PFqElement a, PFqElement b) {
    FqElement tmp;
    Fq_inv(&tmp, b);
    Fq_mul(r, a, &tmp);
}

void Fq_fail() {
    assert(false);
}


RawFq::RawFq() {
    Fq_init();
    set(fZero, 0);
    set(fOne, 1);
    neg(fNegOne, fOne);
}

RawFq::~RawFq() {
}

void RawFq::fromString(Element &r, const std::string &s, uint32_t radix) {
    mpz_t mr;
    mpz_init_set_str(mr, s.c_str(), radix);
    mpz_fdiv_r(mr, mr, q);
    for (int i=0; i<Fq_N64; i++) r.v[i] = 0;
    mpz_export((void *)(r.v), NULL, -1, 8, -1, 0, mr);
    Fq_rawToMontgomery(r.v,r.v);
    mpz_clear(mr);
}

void RawFq::fromUI(Element &r, unsigned long int v) {
    mpz_t mr;
    mpz_init(mr);
    mpz_set_ui(mr, v);
    for (int i=0; i<Fq_N64; i++) r.v[i] = 0;
    mpz_export((void *)(r.v), NULL, -1, 8, -1, 0, mr);
    Fq_rawToMontgomery(r.v,r.v);
    mpz_clear(mr);
}

RawFq::Element RawFq::set(int value) {
  Element r;
  set(r, value);
  return r;
}

void RawFq::set(Element &r, int value) {
  mpz_t mr;
  mpz_init(mr);
  mpz_set_si(mr, value);
  if (value < 0) {
      mpz_add(mr, mr, q);
  }

  mpz_export((void *)(r.v), NULL, -1, 8, -1, 0, mr);
      
  for (int i=0; i<Fq_N64; i++) r.v[i] = 0;
  mpz_export((void *)(r.v), NULL, -1, 8, -1, 0, mr);
  Fq_rawToMontgomery(r.v,r.v);
  mpz_clear(mr);
}

std::string RawFq::toString(const Element &a, uint32_t radix) {
    Element tmp;
    mpz_t r;
    Fq_rawFromMontgomery(tmp.v, a.v);
    mpz_init(r);
    mpz_import(r, Fq_N64, -1, 8, -1, 0, (const void *)(tmp.v));
    char *res = mpz_get_str (0, radix, r);
    mpz_clear(r);
    std::string resS(res);
    free(res);
    return resS;
}

void RawFq::inv(Element &r, const Element &a) {
    mpz_t mr;
    mpz_init(mr);
    mpz_import(mr, Fq_N64, -1, 8, -1, 0, (const void *)(a.v));
    mpz_invert(mr, mr, q);


    for (int i=0; i<Fq_N64; i++) r.v[i] = 0;
    mpz_export((void *)(r.v), NULL, -1, 8, -1, 0, mr);

    Fq_rawMMul(r.v, r.v,Fq_rawR3);
    mpz_clear(mr);
}

void RawFq::div(Element &r, const Element &a, const Element &b) {
    Element tmp;
    inv(tmp, b);
    mul(r, a, tmp);
}

#define BIT_IS_SET(s, p) (s[p>>3] & (1 << (p & 0x7)))
void RawFq::exp(Element &r, const Element &base, uint8_t* scalar, unsigned int scalarSize) {
    bool oneFound = false;
    Element copyBase;
    copy(copyBase, base);
    for (int i=scalarSize*8-1; i>=0; i--) {
        if (!oneFound) {
            if ( !BIT_IS_SET(scalar, i) ) continue;
            copy(r, copyBase);
            oneFound = true;
            continue;
        }
        square(r, r);
        if ( BIT_IS_SET(scalar, i) ) {
            mul(r, r, copyBase);
        }
    }
    if (!oneFound) {
        copy(r, fOne);
    }
}

void RawFq::toMpz(mpz_t r, const Element &a) {
    Element tmp;
    Fq_rawFromMontgomery(tmp.v, a.v);
    mpz_import(r, Fq_N64, -1, 8, -1, 0, (const void *)tmp.v);
}

void RawFq::fromMpz(Element &r, const mpz_t a) {
    for (int i=0; i<Fq_N64; i++) r.v[i] = 0;
    mpz_export((void *)(r.v), NULL, -1, 8, -1, 0, a);
    Fq_rawToMontgomery(r.v, r.v);
}

int RawFq::toRprBE(const Element &element, uint8_t *data, int bytes)
{
    if (bytes < Fq_N64 * 8) {
      return -(Fq_N64 * 8);
    }

    mpz_t r;
    mpz_init(r);
  
    toMpz(r, element);
    
    mpz_export(data, NULL, 1, 8, 1, 0, r);
  
    return Fq_N64 * 8;
}

int RawFq::fromRprBE(Element &element, const uint8_t *data, int bytes)
{
    if (bytes < Fq_N64 * 8) {
      return -(Fq_N64* 8);
    }
    mpz_t r;
    mpz_init(r);

    mpz_import(r, Fq_N64 * 8, 0, 1, 0, 0, data);
    fromMpz(element, r);
    return Fq_N64 * 8;
}

static bool init = Fq_init();

RawFq RawFq::field;

