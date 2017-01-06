#ifndef GAUSS_LEGENDRE_QUAD_H
#define GAUSS_LEGENDRE_QUAD_H

struct gauss_quad_info {
    int n;
    double *x, *weight;
};

extern const struct gauss_quad_info *gauss_rule(int rule_index);
extern double gauss_rule_abscissa(const struct gauss_quad_info *rule, int i);
extern double gauss_rule_weigth(const struct gauss_quad_info *rule, int i);

#define UNIT_RULE 0
#define GAUSS_LEGENDRE_RULE_3 1
#define GAUSS_LEGENDRE_RULE_5 2
#define GAUSS_LEGENDRE_RULE_7 3

#endif
