import sys
from sympy import *
from string import Template

import tauc_lorentz

def tauc_lorentz_diff_epsilon_cse():
    eg, e0, c, en, a = symbols('eg e0 c en a')
    eps1, eps2 = tauc_lorentz.epsilon()
    cse_expr_list = [eps1, eps2]
    diff_variables = [eg, a, e0, c]
    eps1der = [diff(eps1, var) for var in diff_variables]
    eps2der = [diff(eps2, var) for var in diff_variables]
    return cse(cse_expr_list + eps1der + eps2der, optimizations='basic')

xdefs, xexprs = tauc_lorentz_diff_epsilon_cse()

def inverse_transform(a_p, e0_p, c_p):
    e0 = (e0_p**4 + c_p**4 / 4)**Rational(1, 4)
    c = sqrt(2 * e0**2 - 2 * e0_p**2)
    a = a_p * c_p**4 / (4 * e0 * c)
    return (a, e0, c)

def parameters_cse():
    a_p, e0_p, c_p = symbols('a_p e0_p c_p')
    a, e0, c = inverse_transform(a_p, e0_p, c_p)

    derivatives = [diff(a, a_p), diff(a, e0_p), diff(a, c_p), diff(e0, a_p), diff(e0, e0_p), diff(e0, c_p), diff(c, a_p), diff(c, e0_p), diff(c, c_p)]
    return cse([simplify(expr) for expr in derivatives])

dnames = ["dp%d%d" % (i,j) for i in range(1, 4) for j in range(1, 4)]

xdefs_p, xexprs_p = parameters_cse()


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
    eps1 = xe_cxx[0], eps2 = xe_cxx[1],
    der_eps1_eg = xe_cxx[2], der_eps1_a = xe_cxx[3], der_eps1_e0 = xe_cxx[4], der_eps1_c = xe_cxx[5],
    der_eps2_eg = xe_cxx[6], der_eps2_a = xe_cxx[7], der_eps2_e0 = xe_cxx[8], der_eps2_c = xe_cxx[9],
    parameters_defs = format_definitions(xdefs_p),
    parameters_jacob = "\n".join(["const double %s = %s;" % (dnames[i], cxxcode(expr)) for i, expr in enumerate(xexprs_p)])
))
