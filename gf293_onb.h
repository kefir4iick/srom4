#ifndef GF293_ONB_H
#define GF293_ONB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define GF_M     293
#define GF_WORDS ((GF_M + 31) / 32)

typedef struct {
    uint32_t words[GF_WORDS];
} GF293;

GF293 gf_zero_onb(void);
GF293 gf_one_onb(void);

bool  gf_is_zero_onb(const GF293* a);
bool  gf_is_one_onb(const GF293* a);

GF293  gf_add_onb(const GF293* a, const GF293* b);
GF293  gf_mul_onb(const GF293* u, const GF293* v);
GF293  gf_square_onb(const GF293* a);
void   gf_square_onb_inplace(GF293* a);

GF293  gf_pow_onb(const GF293* a, const GF293* exp);
GF293  gf_inv_onb(const GF293* a);

uint8_t gf_trace_onb(const GF293* a);

bool gf_to_bitstr_onb(const GF293* a, char* out, size_t out_len);
bool gf_from_bitstr_onb(const char* in, GF293* out);

bool gf_to_hex_onb(const GF293* a, char* out, size_t out_len);
bool gf_from_hex_onb(const char* hex, GF293* out);

#endif 
