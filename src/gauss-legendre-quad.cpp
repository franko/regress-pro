#include "gauss-legendre-quad.h"

/* Legendre-Gauss quadrature abscissae and coefficients. From:
   http://pomax.github.io/bezierinfo/legendre-gauss.html .
   The abscissa and weight are implicit for the central point. */
static double gauss_quad_3_x[1] = {0.7745966692414834};
static double gauss_quad_3_w[1] = {0.5555555555555556};

static double gauss_quad_5_x[2] = {0.5384693101056831, 0.9061798459386640};
static double gauss_quad_5_w[2] = {0.4786286704993665, 0.2369268850561891};

static double gauss_quad_7_x[3] = {0.4058451513773972, 0.7415311855993945, 0.9491079123427585};
static double gauss_quad_7_w[3] = {0.3818300505051189, 0.2797053914892766, 0.1294849661688697};

static struct gauss_quad_info gauss_quad_rules[] = {
    {0, 0, 0},
    {1, gauss_quad_3_x, gauss_quad_3_w},
    {2, gauss_quad_5_x, gauss_quad_5_w},
    {3, gauss_quad_7_x, gauss_quad_7_w},
};

const struct gauss_quad_info *gauss_rule(int rule_index) {
    return &gauss_quad_rules[rule_index];
}

/* The variable i should vary between -n and n where n is rule->n. */
double gauss_rule_abscissa(const struct gauss_quad_info *rule, int i) {
    return (i < 0 ? - rule->x[-i-1] : (i > 0 ? rule->x[i-1] : 0.0));
};

double gauss_rule_weigth(const struct gauss_quad_info *rule, int i) {
    if (i == 0) {
        double sum = 0.0;
        for (int j = 0; j < rule->n; j++) {
            sum += rule->weight[j];
        }
        return 2.0 - 2.0 * sum;
    }
    return rule->weight[i > 0 ? i-1 : -i-1];
};
