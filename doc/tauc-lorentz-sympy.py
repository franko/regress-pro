from sympy import *

eg, e0, c, en, a = symbols('eg e0 c en a')
eps_inf = symbols('eps_inf')

alpha2 = 4*e0**2 - c**2
alpha = sqrt(alpha2)

gamma2 = e0**2 - c**2/2
gamma = sqrt(gamma2)

a_ln = (eg**2 - e0**2) * en**2 + eg**2 * c**2 - e0**2 * (e0**2 + 3 * eg**2)
a_tan = (en**2 - e0**2) * (e0**2 + eg**2) + eg**2 * c**2
csi4 = (en**2 - gamma2)**2 + alpha2 * c**2 / 4

# first epsilon_1 term
eps1_1 = (a * c) / (pi * csi4) * a_ln / (2 * alpha * e0) * log((e0**2 + eg**2 + alpha * eg) / (e0**2 + eg**2 - alpha * eg))

# 2nd term
eps1_2 = -a / (pi * csi4) * a_tan / e0 * (pi - atan((2 * eg + alpha) / c) + atan((- 2 * eg + alpha) / c))

eps1_3 = 2 * (a * e0) / (pi * csi4 * alpha) * eg * (en**2 - gamma2) * (pi + 2 * atan(2 * (gamma2 - eg**2) / (alpha * c)))

eps1_4 = - (a * e0 * c) / (pi * csi4) * (en**2 + eg**2) / en * log(abs(en - eg) / (en + eg))

eps1_5 = (2 * a * e0 * c) / (pi * csi4) * eg * log((abs(en - eg) * (en + eg)) / sqrt((e0**2 - eg**2)**2 + eg**2 * c**2))

# epsilon_1 obtained summing all the terms + eps_inf
eps1 = eps_inf + eps1_1 + eps1_2 + eps1_3 + eps1_4 + eps1_5

# epsilon_2, imaginary part of epsilon
eps2 = elementary.piecewise.Piecewise(((a * e0 * c * (en - eg)**2) / ((en**2 - e0**2)**2 + c**2 * en**2) * 1 / en, en > eg), (0, True))

xdefs, xexprs = cse([eps1_1, eps1_2, eps1_3, eps1_4, eps1_5, eps2])
# xdefs, xexprs = cse([eps1_1, eps1_2, eps1_3, eps1_4, eps1_5, diff(eps1_1, a)])

cxxcode = printing.cxxcode

print("DEFINITIONS:")
for s, val in xdefs:
    print("double %s = %s;" % (cxxcode(s), cxxcode(val)))
print("")

print("EXPRESSIONS:")
for s, val in zip(["eps1_1", "eps1_2", "eps1_3", "eps1_4", "eps1_5", "eps2"], xexprs):
    print("double %s = %s;" % (cxxcode(s), cxxcode(val)))
print("")

# Print C++11 code
# print(printing.cxxcode(simplify(diff(eps1_1, a))))

# Example nitride
nit_parameters = [(eps_inf, 1), (eg, 4.25), (a, 110), (e0, 9.5), (c, 3.4)]
def nitride_eval(expr, wl):
    return expr.subs(nit_parameters).subs(en, 1240 / wl)

wl0 = 200.0
print("a_ln    (%g) = %g" % (wl0, nitride_eval(a_ln, wl0)))
print("a_tan   (%g) = %g" % (wl0, nitride_eval(a_tan, wl0)))
print("alpha2  (%g) = %g" % (wl0, nitride_eval(alpha2, wl0)))
print("gamma2  (%g) = %g" % (wl0, nitride_eval(gamma2, wl0)))
print("csi4    (%g) = %g" % (wl0, nitride_eval(csi4, wl0)))
print("")

print("eps1_1(%g) = %g" % (wl0, nitride_eval(eps1_1, wl0)))
print("eps1_2(%g) = %g" % (wl0, nitride_eval(eps1_2, wl0)))
print("eps1_3(%g) = %g" % (wl0, nitride_eval(eps1_3, wl0)))
print("eps1_4(%g) = %g" % (wl0, nitride_eval(eps1_4, wl0)))
print("eps1_5(%g) = %g" % (wl0, nitride_eval(eps1_5, wl0)))
print("")

nit_n = sqrt(nitride_eval(eps1, wl0) - I * nitride_eval(eps2, wl0));
print("n(%g) = %g, k(%g) = %g" % (wl0, re(nit_n), wl0, im(nit_n)))

# print("eps1  (%g) = %g" % (wl0, nitride_eval(eps1,   wl0)))
# print("eps2  (%g) = %g" % (wl0, nitride_eval(eps2,   wl0)))

wl = symbols('wl')

# plot(nitride(eps2), (wl, 190, 800))
plot(nitride_eval(eps1, wl), (wl, 190, 800))

# plot((re(sqrt(nitride(eps1) - I * nitride(eps2))), (wl, 190, 800)))
# plot((-im(sqrt(nitride(eps1) - I * nitride(eps2))), (wl, 190, 800)))
