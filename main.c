#include <stdio.h>
#include <string.h>
#include "gf293_onb.h"

static void print_hex(const char* label, const GF293* x) {
    char hex[80];
    gf_to_hex_onb(x, hex, sizeof(hex));
    printf("%s%s\n", label, hex);
}

int main(void) {
    GF293 A, B, C, R;
    char hex[80];

    const char* A_hex =
        "6F8C2A91D3B7E4F05A9C1E7D4B20F8A63C9D5712E0A4B6F93D8C2E51A7B4096D3C1F8";

    const char* B_hex =
        "1A4E9C7D0F3B6A258E41C9D52F7086AB3C4D91E7F8A20B65C3D94E2A1F6B0875C9";

    const char* C_hex =
        "3D7A1F0B9C2E5486A17F3C9D0E5B2A1C4F8E6D3A9B7012C5D4E9A83F1B6C0D25E7";

    if (!gf_from_hex_onb(A_hex, &A) ||
        !gf_from_hex_onb(B_hex, &B) ||
        !gf_from_hex_onb(C_hex, &C)) {
        printf("error hex\n");
        return 1;
    }

    printf("A (hex, ONB) = %s\n", A_hex);
    printf("B (hex, ONB) = %s\n", B_hex);
    printf("C (hex, ONB) = %s\n\n", C_hex);

    R = gf_add_onb(&A, &B);
    gf_to_hex_onb(&R, hex, sizeof(hex));
    printf("A + B = %s\n\n", hex);

    R = gf_mul_onb(&A, &B);
    gf_to_hex_onb(&R, hex, sizeof(hex));
    printf("A * B = %s\n\n", hex);

    R = gf_square_onb(&A);
    gf_to_hex_onb(&R, hex, sizeof(hex));
    printf("A^2 = %s\n\n", hex);

    R = gf_pow_onb(&A, &B);
    gf_to_hex_onb(&R, hex, sizeof(hex));
    printf("A^B = %s\n\n", hex);

    GF293 invA = gf_inv_onb(&A);
    GF293 check = gf_mul_onb(&A, &invA);

    gf_to_hex_onb(&invA, hex, sizeof(hex));
    printf("inv(A) = %s\n", hex);

    gf_to_hex_onb(&check, hex, sizeof(hex));
    printf("A * inv(A) = %s\n\n", hex);

    printf("Tr(A) = %u\n\n", gf_trace_onb(&A));

    gf_to_hex_onb(&A, hex, sizeof(hex));
    printf("A -> HEX -> A = %s\n\n", hex);

    GF293 AA1 = gf_mul_onb(&A, &A);
    GF293 AA2 = gf_square_onb(&A);

    char h1[80], h2[80];
    gf_to_hex_onb(&AA1, h1, sizeof h1);
    gf_to_hex_onb(&AA2, h2, sizeof h2);

    printf("A * A = %s\n", h1);
    printf("A^2   = %s\n\n", h2);

    GF293 BpC  = gf_add_onb(&B, &C);
    GF293 left = gf_mul_onb(&A, &BpC);

    GF293 AB   = gf_mul_onb(&A, &B);
    GF293 AC   = gf_mul_onb(&A, &C);
    GF293 right= gf_add_onb(&AB, &AC);

    print_hex("B + C     = ", &BpC);
    print_hex("A*(B+C)   = ", &left);
    print_hex("A*B       = ", &AB);
    print_hex("A*C       = ", &AC);
    print_hex("A*B + A*C = ", &right);

    GF293 AB1   = gf_mul_onb(&A, &B);
    GF293 left2 = gf_mul_onb(&AB1, &C);

    GF293 BC1   = gf_mul_onb(&B, &C);
    GF293 right2= gf_mul_onb(&A, &BC1);

    print_hex("(A*B)*C = ", &left2);
    print_hex("A*(B*C) = ", &right2);

    return 0;
}
