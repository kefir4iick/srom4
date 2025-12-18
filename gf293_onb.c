#include "gf293_onb.h"

#include <string.h>
#include <assert.h>
#include <stdatomic.h>



static inline uint32_t get_bit(const GF293* a, int pos) {
    return (a->words[pos >> 5] >> (pos & 31)) & 1u;
}

static inline void set_bit(GF293* a, int pos) {
    a->words[pos >> 5] |= (1u << (pos & 31));
}



GF293 gf_zero_onb(void) {
    GF293 z;
    memset(&z, 0, sizeof z);
    return z;
}

GF293 gf_one_onb(void) {
    GF293 r;
    for (int i = 0; i < GF_WORDS; i++)
        r.words[i] = 0xFFFFFFFFu;

    int rem = GF_M & 31;
    if (rem)
        r.words[GF_WORDS - 1] &= (1u << rem) - 1u;

    return r;
}

bool gf_is_zero_onb(const GF293* a) {
    assert(a);
    for (int i = 0; i < GF_WORDS; i++)
        if (a->words[i])
            return false;
    return true;
}

bool gf_is_one_onb(const GF293* a) {
    assert(a);
    GF293 o = gf_one_onb();
    return memcmp(a, &o, sizeof o) == 0;
}



GF293 gf_add_onb(const GF293* a, const GF293* b) {
    assert(a && b);
    GF293 r;
    for (int i = 0; i < GF_WORDS; i++)
        r.words[i] = a->words[i] ^ b->words[i];
    return r;
}



void gf_square_onb_inplace(GF293* a) {
    assert(a);

    uint32_t msb = get_bit(a, GF_M - 1);
    uint32_t carry = 0;

    for (int i = 0; i < GF_WORDS; i++) {
        uint32_t w = a->words[i];
        a->words[i] = (w << 1) | carry;
        carry = w >> 31;
    }

    int rem = GF_M & 31;
    if (rem)
        a->words[GF_WORDS - 1] &= (1u << rem) - 1u;

    if (msb)
        set_bit(a, 0);
}



GF293 gf_square_onb(const GF293* a) {
    assert(a);
    GF293 r = *a;
    gf_square_onb_inplace(&r);
    return r;
}



uint8_t gf_trace_onb(const GF293* a) {
    assert(a);
    uint32_t x = 0;
    for (int i = 0; i < GF_WORDS; i++)
        x ^= a->words[i];

    x ^= x >> 16;
    x ^= x >> 8;
    x ^= x >> 4;
    x ^= x >> 2;
    x ^= x >> 1;
    return (uint8_t)(x & 1u);
}



#define P_ONB    (2 * GF_M + 1)
#define LAM_NNZ (2 * GF_M - 1)

typedef struct {
    uint16_t i, j;
} Pair;

static Pair lambda_pairs[LAM_NNZ];
static atomic_bool lambda_ready = ATOMIC_VAR_INIT(false);

static inline int modp(int x) {
    x %= P_ONB;
    return (x < 0) ? x + P_ONB : x;
}

static void build_lambda_pairs(void) {
    bool expected = false;
    if (!atomic_compare_exchange_strong(&lambda_ready, &expected, true))
        return;

    int pow2[GF_M];
    pow2[0] = 1;

    for (int i = 1; i < GF_M; i++)
        pow2[i] = (pow2[i - 1] << 1) % P_ONB;

    uint8_t seen[(GF_M * GF_M + 7) / 8] = {0};
    int cnt = 0;

    for (int i = 0; i < GF_M; i++) {
        for (int j = 0; j < GF_M; j++) {
            int a = pow2[i];
            int b = pow2[j];

            if (modp( a + b) == 1 ||
                modp( a - b) == 1 ||
                modp(-a + b) == 1 ||
                modp(-a - b) == 1) {

                int idx = i * GF_M + j;
                if (!(seen[idx >> 3] & (1u << (idx & 7)))) {
                    seen[idx >> 3] |= (1u << (idx & 7));
                    assert(cnt < LAM_NNZ);
                    lambda_pairs[cnt++] = (Pair){ i, j };
                }
            }
        }
    }

    assert(cnt == LAM_NNZ);
}

GF293 gf_mul_onb(const GF293* u, const GF293* v) {
    assert(u && v);
    build_lambda_pairs();

    GF293 z = gf_zero_onb();

    for (int i = 0; i < GF_M; i++) {
        uint32_t acc = 0;

        for (int k = 0; k < LAM_NNZ; k++) {
            int r = i + lambda_pairs[k].i;
            int c = i + lambda_pairs[k].j;
            if (r >= GF_M) r -= GF_M;
            if (c >= GF_M) c -= GF_M;

            acc ^= get_bit(u, r) & get_bit(v, c);
        }

        if (acc & 1u)
            set_bit(&z, i);
    }
    
    int rem = GF_M & 31;
    if (rem)
      z.words[GF_WORDS - 1] &= (1u << rem) - 1u;

    return z;
}



GF293 gf_pow_onb(const GF293* a, const GF293* exp) {
    assert(a && exp);

    GF293 r = gf_one_onb();
    for (int i = GF_M - 1; i >= 0; i--) {
        gf_square_onb_inplace(&r);
        if (get_bit(exp, i))
            r = gf_mul_onb(&r, a);
    }
    return r;
}

GF293 gf_inv_onb(const GF293* a) {
    assert(a);

    if (gf_is_zero_onb(a))
        return gf_zero_onb();

    GF293 t = *a;
    GF293 inv = gf_one_onb();

    for (int i = 1; i < GF_M; i++) {
        gf_square_onb_inplace(&t);
        inv = gf_mul_onb(&inv, &t);
    }
    return inv;
}











bool gf_to_bitstr_onb(const GF293* a, char* out, size_t out_len) {
    assert(a && out);
    if (out_len < GF_M + 1)
        return false;

    for (int i = 0; i < GF_M; i++) {
        int pos = GF_M - 1 - i;
        out[i] = get_bit(a, pos) ? '1' : '0';
    }
    out[GF_M] = '\0';
    return true;
}

bool gf_from_bitstr_onb(const char* in, GF293* out) {
    assert(in && out);

    GF293 r = gf_zero_onb();

    for (int i = 0; i < GF_M; i++) {
        if (in[i] == '1') {
            int pos = GF_M - 1 - i;
            set_bit(&r, pos);
        } else if (in[i] != '0') {
            return false;
        }
    }

    if (in[GF_M] != '\0')
        return false;

    *out = r;
    return true;
}


static inline int hex_val(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return c - 'a' + 10;
    if ('A' <= c && c <= 'F') return c - 'A' + 10;
    return -1;
}

bool gf_from_hex_onb(const char* hex, GF293* out) {
    assert(hex && out);

    size_t len = strlen(hex);
    size_t hex_len = (GF_M + 3) / 4;
    if (len == 0 || len > hex_len)
        return false;

    GF293 r = gf_zero_onb();
    size_t shift = hex_len - len;

    for (size_t i = 0; i < len; i++) {
        int v = hex_val(hex[i]);
        if (v < 0)
            return false;

        for (int b = 0; b < 4; b++) {
            size_t idx = (i + shift) * 4 + b;
            if (idx >= GF_M)
                continue;

            if (v & (1 << (3 - b))) {
                int pos = GF_M - 1 - idx;
                set_bit(&r, pos);
            }
        }
    }

    *out = r;
    return true;
}

bool gf_to_hex_onb(const GF293* a, char* out, size_t out_len) {
    assert(a && out);

    size_t hex_len = (GF_M + 3) / 4;
    if (out_len < hex_len + 1)
        return false;

    static const char* H = "0123456789ABCDEF";
    char tmp[80];

    for (size_t i = 0; i < hex_len; i++) {
        int v = 0;
        for (int b = 0; b < 4; b++) {
            size_t idx = i * 4 + b;
            if (idx >= GF_M)
                continue;

            int pos = GF_M - 1 - idx;
            v |= get_bit(a, pos) << (3 - b);
        }
        tmp[i] = H[v];
    }

    tmp[hex_len] = '\0';

    size_t start = 0;
    while (start + 1 < hex_len && tmp[start] == '0')
        start++;

    strcpy(out, tmp + start);
    return true;
}
