from sympy import *

def inverse_transform(a_p, e0_p, c_p):
    e0 = (e0_p**4 + c_p**4 / 4)**Rational(1, 4)
    c = sqrt(2 * e0**2 - 2 * e0_p**2)
    a = a_p * c_p**4 / (4 * e0 * c)
    return (a, e0, c)

def direct_transform(a, e0, c):
    Gqq = (4 * e0**2 - c**2) * c**2
    G = Gqq**Rational(1, 4)
    Ep = sqrt(e0**2 - c**2 / 2)
    ALp = (4 * a * e0 * c) / Gqq
    return (ALp, Ep, G)

a_p, e0_p, c_p = symbols('a_p e0_p c_p')
a, e0, c = inverse_transform(a_p, e0_p, c_p)

cxxcode = printing.cxxcode

cook = lambda expr: cxxcode(simplify(expr))

# print("dA'/dA   = ", cook(diff(a_p, a)))
# print("dA'/dE0  = ", cook(diff(a_p, e0)))
# print("dA'/dC   = ", cook(diff(a_p, c)))

# print("dE0'/dA  = ", cook(diff(e0_p, a)))
# print("dE0'/dE0 = ", cook(diff(e0_p, e0)))
# print("dE0'/dC  = ", cook(diff(e0_p, c)))

# print("dC'/dA   = ", cook(diff(c_p, a)))
# print("dC'/dE0  = ", cook(diff(c_p, e0)))
# print("dC'/dC   = ", cook(diff(c_p, c)))

derivatives = [diff(a, a_p), diff(a, e0_p), diff(a, c_p), diff(e0, a_p), diff(e0, e0_p), diff(e0, c_p), diff(c, a_p), diff(c, e0_p), diff(c, c_p)]
dnames = ["dp%d%d" % (i,j) for i in range(1, 4) for j in range(1, 4)]
xdefs, xexprs = cse([simplify(expr) for expr in derivatives])

definitions= "\n".join(["const auto %s = %s;" % (cxxcode(s), cxxcode(val)) for s, val in xdefs])

print(definitions)
for i, expr in enumerate(xexprs):
    print("const double %s = %s;" % (dnames[i], cxxcode(expr)))

# print("const double %s = %s;" % ("da_dap  ", cook(diff(a, a_p))))
# print("const double %s = %s;" % ("da_de0p ", cook(diff(a, e0_p))))
# print("const double %s = %s;" % ("da_dcp  ", cook(diff(a, c_p))))

# print("const double %s = %s;" % ("de0_dap ", cook(diff(e0, a_p))))
# print("const double %s = %s;" % ("de0_de0p", cook(diff(e0, e0_p))))
# print("const double %s = %s;" % ("de0_dcp ", cook(diff(e0, c_p))))

# print("const double %s = %s;" % ("dc_dap  ", cook(diff(c, a_p))))
# print("const double %s = %s;" % ("dc_de0p ", cook(diff(c, e0_p))))
# print("const double %s = %s;" % ("dc_dcp  ", cook(diff(c, c_p))))
