from sympy import *

# Compute symbolically the epsilon1 and 2 (real and imaginary parts) for
# the Kramers oscillator model.
def epsilon():
    a, en, eg, phi, e = symbols('a en eg phi, e', real=True)

    lor_ker = a / (en**2 - e**2 + I * eg * e)
    # First order Lorentzian term, using normalizing factor eg / 2.
    lor_ker_first = (eg / 2) * diff(lor_ker, en)
    lor_ker_phi = cos(phi) * lor_ker + sin(phi) * lor_ker_first

    return (re(lor_ker_phi), -im(lor_ker_phi))

