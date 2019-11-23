from sympy import *

# Compute symbolically the epsilon1 and 2 (real and imaginary parts) for
# the Kramers oscillator model.
def epsilon():
    a, en, eg, phi, e = symbols('a en eg phi, e')
    egp = en

    lor_d  = (en**2 - e**2)**2 + eg**2 * e**2
    lor_dp = (en**2 - e**2)**2 + egp**2 * e**2

    # first multiplicative term, real and imaginary part
    # \frac{N}{E_n^2 - E^2 + i E_g E}
    eps_f1_re = a * (en**2 - e**2) / lor_d
    eps_f1_im = - a * eg * e / lor_d

    # second multiplicative term, real and imaginary part
    # \cos \phi + \sin \phi \, \frac{1}{2} \, \frac{E_n^2 + E^2}{E_n^2 - E^2 + i E_n E}
    f = (en**2 + e**2) / (2 * lor_dp)
    eps_f2_re = cos(phi) + sin(phi) * f * (en**2 - e**2)
    eps_f2_im = - sin(phi) * f * egp * e

    # We calculate now the real part of the epsilon by taking the product
    # of the two complex terms.
    eps_re = eps_f1_re * eps_f2_re - eps_f1_im * eps_f2_im
    eps_im = eps_f1_re * eps_f2_im + eps_f1_im * eps_f2_re

    return (eps_re, -eps_im)
