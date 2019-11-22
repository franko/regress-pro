import sys
from sympy import *
from string import Template

import kramers_model

def kramers_epsilon_cse():
    eps1, eps2 = kramers_model.epsilon()
    return cse([eps1, eps2], optimizations='basic')

xdefs, xexprs = kramers_epsilon_cse()

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
    eps2 = xe_cxx[1]
))
