import sys
from sympy import *
from string import Template

import kramers

def kramers_diff_epsilon_cse():
    a, en, eg, phi = symbols('a, en, eg, phi')
    eps1, eps2 = kramers.epsilon()
    cse_expr_list = [eps1, eps2]
    diff_variables = [a, en, eg, phi]
    eps1der = [diff(eps1, var) for var in diff_variables]
    eps2der = [diff(eps2, var) for var in diff_variables]
    return cse(cse_expr_list + eps1der + eps2der, optimizations='basic')

xdefs, xexprs = kramers_diff_epsilon_cse()

if len(sys.argv) != 2:
    print("Usage: %s <filename>" % sys.argv[0])
    exit(1)

code_header = "/* Generated with sympy using the command: %s */\n" % " ".join(sys.argv)

template_filename = sys.argv[1]
template_file = open(template_filename, 'r')
template_code = code_header + template_file.read()
template_file.close()

cxxcode = printing.cxxcode

xe_cxx = [cxxcode(expr) for expr in xexprs]

def format_definitions(defs):
    return "\n".join(["const auto %s = %s;" % (cxxcode(s), cxxcode(val)) for s, val in defs])

print(Template(template_code).substitute(
    epsilon_defs= format_definitions(xdefs),
    eps1 = xe_cxx[0],
    eps2 = xe_cxx[1],
    der_eps1_a   = xe_cxx[2],
    der_eps1_en  = xe_cxx[3],
    der_eps1_eg  = xe_cxx[4],
    der_eps1_phi = xe_cxx[5],
    der_eps2_a   = xe_cxx[6],
    der_eps2_en  = xe_cxx[7],
    der_eps2_eg  = xe_cxx[8],
    der_eps2_phi = xe_cxx[9],
))
