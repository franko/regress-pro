import sys
from sympy import *

from string import Template

def tauc_lorentz_epsilon_cse():
    eg, e0, c, en, a = symbols('eg e0 c en a')
    eps_inf = symbols('eps_inf')
    eg_delta = symbols('eg_delta')

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
    eps1_1 = (a * c) / (pi * csi4) * a_ln / (2 * alpha * e0) * log((e0**2 + eg**2 + alpha * eg) / (e0**2 + eg**2 - alpha * eg))

    # 2nd term
    eps1_2 = -a / (pi * csi4) * a_tan / e0 * (pi - atan((2 * eg + alpha) / c) + atan((- 2 * eg + alpha) / c))

    eps1_3 = 2 * (a * e0) / (pi * csi4 * alpha) * eg * (en**2 - gamma2) * (pi + 2 * atan(2 * (gamma2 - eg**2) / (alpha * c)))

    # First 3 terms of epsilon1 when alpha is imaginary
    eps1_alpha_imag_1 = (2 * eg * a * c * a_ln) / (2 * pi * csi4 * e0 * (e0**2 + eg**2))
    eps1_alpha_imag_2 = - (a * a_tan) / (pi * csi4 * e0) * (2 * atan(c / (2 * eg)))
    eps1_alpha_imag_3 = (2 * a * e0 * eg * (en**2 - gamma2)) / (pi * csi4) * (c / (e0**2 + eg**2))

    # Compose epsilon 1 to 3 terms using different expressions depending if alpha is real or imaginary.
    eps1ts.append(Piecewise((eps1_1 + eps1_2 + eps1_3, c < (2 - 1e-10) * e0), (eps1_alpha_imag_1 + eps1_alpha_imag_2 + eps1_alpha_imag_3, True)))

    eps1_4 = - (a * e0 * c) / (pi * csi4) * (en**2 + eg**2) / en * (log_abs_en_eg - log(en + eg))
    eps1_5 = (2 * a * e0 * c) / (pi * csi4) * eg * (log_abs_en_eg + log(en + eg) - log(sqrt((e0**2 - eg**2)**2 + eg**2 * c**2)))

    # limiting term replacing term4 + term5 of epsilon1 when E -> Eg
    eps45_limit = (2 * a * en * e0 * c) / (pi * ((en**2 - gamma2)**2 + alpha2 * c**2 / 4)) * log((4 * en**2) / sqrt((e0**2 - eg**2)**2 + eg**2 * c**2))

    eps1ts.append(Piecewise((eps45_limit, (en < eg + eg_delta) & (en > eg - eg_delta)), (eps1_4 + eps1_5, True)))

    # epsilon_1 obtained summing all the terms EXCLUDING eps_inf
    eps1 = sum(eps1ts)

    # epsilon_2, imaginary part of epsilon
    eps2_ker = (a * e0 * c * (en - eg)**2) / ((en**2 - e0**2)**2 + c**2 * en**2) * 1 / en
    eps2 = Piecewise((eps2_ker, en > eg), (0, True))

    return cse([eps1, eps2])

xdefs, xexprs = tauc_lorentz_epsilon_cse()

if len(sys.argv) != 2:
    print("Usage: %s <filename>" % sys.argv[0])
    exit(1)

code_header = "/* Generated with sympy using the command: %s */\n" % " ".join(sys.argv)

template_filename = sys.argv[1]
template_file = open(template_filename, 'r')
tauc_lorentz_code = code_header + template_file.read()
template_file.close()

cxxcode = printing.cxxcode

xe_cxx = [cxxcode(expr) for expr in xexprs]

def format_definitions(defs):
    return "\n".join(["const auto %s = %s;" % (cxxcode(s), cxxcode(val)) for s, val in defs])

print(Template(tauc_lorentz_code).substitute(
    epsilon_defs= format_definitions(xdefs),
    eps1 = xe_cxx[0],
    eps2 = xe_cxx[1]
))
