from sympy import *

from string import Template

eg, e0, c, en, a = symbols('eg e0 c en a')
eps_inf = symbols('eps_inf')

alpha2 = 4*e0**2 - c**2
alpha = sqrt(alpha2)

gamma2 = e0**2 - c**2/2
gamma = sqrt(gamma2)

a_ln = (eg**2 - e0**2) * en**2 + eg**2 * c**2 - e0**2 * (e0**2 + 3 * eg**2)
a_tan = (en**2 - e0**2) * (e0**2 + eg**2) + eg**2 * c**2
csi4 = (en**2 - gamma2)**2 + alpha2 * c**2 / 4

eps1ts = []

log_abs_en_eg = Piecewise((log(en - eg), en > eg), (log(eg - en), True))

# first epsilon_1 term
eps1ts.append((a * c) / (pi * csi4) * a_ln / (2 * alpha * e0) * log((e0**2 + eg**2 + alpha * eg) / (e0**2 + eg**2 - alpha * eg)))

# 2nd term
eps1ts.append(-a / (pi * csi4) * a_tan / e0 * (pi - atan((2 * eg + alpha) / c) + atan((- 2 * eg + alpha) / c)))

eps1ts.append(2 * (a * e0) / (pi * csi4 * alpha) * eg * (en**2 - gamma2) * (pi + 2 * atan(2 * (gamma2 - eg**2) / (alpha * c))))

eps1ts.append(- (a * e0 * c) / (pi * csi4) * (en**2 + eg**2) / en * (log_abs_en_eg - log(en + eg)))

eps1ts.append((2 * a * e0 * c) / (pi * csi4) * eg * (log_abs_en_eg + log(en + eg) - log(sqrt((e0**2 - eg**2)**2 + eg**2 * c**2))))

# epsilon_1 obtained summing all the terms EXCLUDING eps_inf
eps1 = sum(eps1ts)

# epsilon_2, imaginary part of epsilon
eps2_ker = (a * e0 * c * (en - eg)**2) / ((en**2 - e0**2)**2 + c**2 * en**2) * 1 / en
eps2 = Piecewise((eps2_ker, en > eg), (0, True))

cse_expr_list = [eps1, eps2]

diff_variables = [eg, a, e0, c]

eps1der = [diff(eps1, var) for var in diff_variables]
eps2der = [diff(eps2, var) for var in diff_variables]

xdefs, xexprs = cse(cse_expr_list + eps1der + eps2der)

cxxcode = printing.cxxcode

tauc_lorentz_code = """
cmpl tauc_lorentz_n_value_deriv(const disp_t *d, double lambda, cmpl_vector *pd) {
    const struct disp_fb *fb = &d->disp.fb;
    const int nb = fb->n;
    double en = TL_EV_NM / lambda;

    /* If Eg is negative use zero instead. A negative Eg is not meaningful. */
    const double eg = (fb->eg >= 0 ? fb->eg : 0.0);

    pd->at(TL_NINF_OFFS) = cmpl(1.0, 0.0);
    pd->at(TL_EG_OFFS) = 0;

    double er_sum = fb->n_inf, ei_sum = 0.0;
    for (int osc_index = TL_OSC_OFFS, k = 0; k < nb; k++, osc_index += TL_NB_OSC_PARAMS) {
        const struct fb_osc *osc = fb->osc + k;
        double a, e0, c;
        if (fb->form == TAUC_LORENTZ_STANDARD) {
            a = osc->a;
            e0 = osc->b;
            c = osc->c;
        } else {
            /* Use rationalized lorentzian coefficients of the abs peak:
               - osc->a is AL' and gives the height of the peak
               - osc->b is Ep and is the energy position
               - osc->c is Gamma and is the peak width. */
            e0 = pow(pow4(osc->b) + pow4(osc->c) / 4, 0.25);
            c = sqrt(2 * pow2(e0) - 2 * pow2(osc->b));
            a = osc->a * pow4(osc->c) / (4 * e0 * c);
        }

        ${definitions}

        er_sum += ${eps1};
        ei_sum += ${eps2};

        const double d_eg_real = ${der_eps1_eg};
        const double d_eg_imag = ${der_eps2_eg};
        pd->at(TL_EG_OFFS) += cmpl(d_eg_real, -d_eg_imag);

        const double d_a_real = ${der_eps1_a};
        const double d_a_imag = ${der_eps2_a};
        pd->at(osc_index + TL_AL_OFFS) = cmpl(d_a_real, -d_a_imag);

        const double d_e0_real = ${der_eps1_e0};
        const double d_e0_imag = ${der_eps2_e0};
        pd->at(osc_index + TL_E0_OFFS) = cmpl(d_e0_real, -d_e0_imag);

        const double d_c_real = ${der_eps1_c};
        const double d_c_imag = ${der_eps2_c};
        pd->at(osc_index + TL_C_OFFS)  = cmpl(d_c_real, -d_c_imag);
    }

    cmpl nn = sqrt(cmpl(er_sum, -ei_sum));

    for (int i = 0; i < TL_NB_GLOBAL_PARAMS + nb * TL_NB_OSC_PARAMS; i++) {
        pd->at(i) = cmpl(1, 0) / (2 * nn) * pd->at(i);
    }

    return nn;
}
"""

xe_cxx = [cxxcode(expr) for expr in xexprs]

print(Template(tauc_lorentz_code).substitute(
    definitions= "\n".join(["const auto %s = %s;" % (cxxcode(s), cxxcode(val)) for s, val in xdefs]),
    eps1 = xe_cxx[0], eps2 = xe_cxx[1],
    der_eps1_eg = xe_cxx[2], der_eps1_a = xe_cxx[3], der_eps1_e0 = xe_cxx[4], der_eps1_c = xe_cxx[5],
    der_eps2_eg = xe_cxx[6], der_eps2_a = xe_cxx[7], der_eps2_e0 = xe_cxx[8], der_eps2_c = xe_cxx[9]
))

# print("GENERATED CODE:")
# for s, val in xdefs:
#     print("double %s = %s;" % (cxxcode(s), cxxcode(val)))
# print("")

# for i, val in enumerate(xexprs):
#     print("EXPR%d: %s;" % (i+1, cxxcode(val)))
# print("")

# Example nitride
nit_parameters = [(eps_inf, 1), (eg, 4.25), (a, 110), (e0, 9.5), (c, 3.4)]
def nitride_eval(expr, wl):
    return expr.subs(nit_parameters).subs(en, 1240 / wl)

def nitride_print_terms(wl):
    print("a_ln    (%g) = %g" % (wl, nitride_eval(a_ln, wl)))
    print("a_tan   (%g) = %g" % (wl, nitride_eval(a_tan, wl)))
    print("alpha2  (%g) = %g" % (wl, nitride_eval(alpha2, wl)))
    print("gamma2  (%g) = %g" % (wl, nitride_eval(gamma2, wl)))
    print("csi4    (%g) = %g" % (wl, nitride_eval(csi4, wl)))
    print("")

    for k in range(0, 5):
        print("eps1[%d](%g) = %g" % (k, wl, nitride_eval(eps1ts[k], wl)))
    print("")

    nit_n = sqrt(nitride_eval(eps1, wl) - I * nitride_eval(eps2, wl));
    print("n(%g) = %g, k(%g) = %g" % (wl, re(nit_n), wl, im(nit_n)))

# nitride_print_terms(200.0)
# nitride_print_terms(240.0)
# nitride_print_terms(633.0)

# print("eps1  (%g) = %g" % (wl0, nitride_eval(eps1,   wl0)))
# print("eps2  (%g) = %g" % (wl0, nitride_eval(eps2,   wl0)))

wl = symbols('wl')

# plot(nitride(eps2), (wl, 190, 800))
# plot(nitride_eval(eps1, wl), (wl, 190, 800))
# plot(nitride_eval(eps2, wl), (wl, 190, 800))

# plot((re(sqrt(nitride(eps1) - I * nitride(eps2))), (wl, 190, 800)))
# plot((-im(sqrt(nitride(eps1) - I * nitride(eps2))), (wl, 190, 800)))
