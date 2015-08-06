#include "number-parse.h"

static int parse_int(const char *text, int *nread)
{
    int n = 0;
    const char *s = text;
    while ((int) *s >= (int) '0' && (int) *s <= (int) '9') {
        int d = (int) *s - (int) '0';
        n = 10 * n + d;
        s ++;
    }
    *nread = s - text;
    return n;
}

static float ipowf(float base, int exp)
{
    float result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }
    return result;
}

const char *skip_spaces(const char *s)
{
    while (*s && *s == ' ') {
        s ++;
    }
    return s;
}

int parse_float(const char *text, unsigned int flags, float *value, int *n_parsed)
{
    const char *s = text;
    if (flags & PARSE_FLOAT_SKIP_SPACES) {
        s = skip_spaces(s);
    }
    int dec_pending = 1;
    int sign = 1;
    if (*s == '+' || *s == '-') {
        sign = (*s == '+' ? 1 : -1);
        s ++;
    }

    int nread;
    int int_part = parse_int(s, &nread);
    if (nread > 0) {
        dec_pending = 0;
    }
    s += nread;

    const char decimal_sym = (flags & PARSE_FLOAT_FRENCH_LOCALE ? ',' : '.');

    int dec_part = 0, dec_power = 0;
    if (*s == decimal_sym) {
        dec_part = parse_int(s + 1, &nread);
        if (nread > 0) {
            dec_pending = 0;
        }
        s += nread + 1;
        dec_power = nread;
    }

    if (dec_pending) return 1;

    int exp_int = 0;
    if (*s == 'e' || *s == 'E') {
        s ++;
        int esign = 1;
        if (*s == '+' || *s == '-') {
            esign = (*s == '+' ? 1 : -1);
            s ++;
        }
        exp_int = esign * parse_int(s, &nread);
        if (nread == 0) return 1;
        s += nread;
    }

    float x = sign * int_part + dec_part / ipowf(10.0, dec_power);
    if (exp_int >= 0) {
        *value = x * ipowf(10.0, exp_int);
    } else if (exp_int < 0) {
        *value = x / ipowf(10.0, - exp_int);
    }

    *n_parsed = s - text;
    return 0;
}
