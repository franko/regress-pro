
/* HODispersion.c
 *
 * Copyright (C) 2005-2011 Francesco Abbate
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <assert.h>
#include <string.h>
#include "dispers.h"
#include "HODispersion.h"

template <typename T>
static inline T sqr(const T x) { return x*x; }

const char *HODispersion::parameter_names[] = {"Nosc", "En", "Eg", "Nu", "Phi"};

/* The HO multiplicative factor is: 16 * Pi * Ry^2 * r0^3 where
   Ry is the Rydberg constant Ry = 13.6058 eV et r0 is the Bohr radius:
   r0 = 0.0529177 nm.
   The definition above would give the following constant:

#define HO_MULT_FACT 1.3788623090164978586499199863586

   For the conversion from eV to nm the value of 1240 is used:

   E(eV) = 1240 / lambda(nm)
*/

#define HO_EV_NM 1240.0

#define HO_MULT_FACT 1.3788623090164978586499199863586

const HODispersionClass ho_dispersion_class{DISP_HO, "Harmonic Oscillator", "ho"};

std::unique_ptr<Dispersion> HODispersionClass::read(Lexer& lexer) const {
    Lexer::quoted_string name;
    int n;
    lexer >> "ho" >> name >> n;
    std::unique_ptr<HODispersion> disp(new HODispersion(name.text(), n));
    for (auto& osc : disp->oscillators()) {
        lexer >> osc.nosc >> osc.en >> osc.eg >> osc.nu >> osc.phi;
    }
    return disp;
}

complex HODispersion::n_value_deriv(double lambda, complex der[]) const {
    const double e = HO_EV_NM / lambda;

    const int nb = m_oscillators.size();
    complex hsum{0.0, 0.0}, hnusum{0.0, 0.0};
    for(int k = 0; k < nb; k++) {
        const Oscillator& osc = m_oscillators[k];

        complex hh = HO_MULT_FACT * osc.nosc * std::exp(- I * osc.phi) / \
             (sqr(osc.en) - sqr(e) + I * osc.eg * e);

        if (der) {
            der[parameter_index(k, NOSC)] = hh / osc.nosc;
            der[parameter_index(k, EN  )] = hh;
            der[parameter_index(k, EG  )] = hh;
            der[parameter_index(k, NU  )] = hh;
            der[parameter_index(k, PHI )] = - I * hh;
        }

        hsum += hh;
        hnusum += osc.nu * hh;
    }

    complex n = std::sqrt(1.0 + hsum/(1.0 - hnusum));
    const bool suppress_k = (n.imag() > 0.0);

    const complex den = 1.0 - hnusum;
    const complex epsfact = 1.0 / (2.0 * std::sqrt(1.0 + hsum / den));

    if (suppress_k) {
        n = complex{n.real(), 0.0};
    }

    if(der == nullptr) {
        return n;
    }

    for(int k = 0; k < nb; k++) {
        const Oscillator& osc = m_oscillators[k];

        const int index_nu = parameter_index(k, NU);
        complex y = hsum / sqr(den) * der[index_nu];
        y *= epsfact;
        der[index_nu] = y;

        const complex dndh = osc.nu * hsum / sqr(den) + 1.0 / den;

        const int index_nosc = parameter_index(k, NOSC);
        y = dndh * der[index_nosc];
        y *= epsfact;
        der[index_nosc] = y;

        const int index_phi = parameter_index(k, PHI);
        y = dndh * der[index_phi];
        y *= epsfact;
        der[index_phi] = y;

        complex hhden = sqr(osc.en) - sqr(e) + I * osc.eg * e;

        const int index_en = parameter_index(k, EN);
        y = dndh * (- 2.0 * osc.en / hhden) * der[index_en];
        y *= epsfact;
        der[index_en] = y;

        const int index_eg = parameter_index(k, EG);
        y = dndh * (- I * e / hhden) * der[index_eg];
        y *= epsfact;
        der[index_eg] = y;
    }

    if(suppress_k) {
        for(int k = 0; k < nb * OSC_PARAMETERS_NUM; k++) {
            const complex y = der[k];
            der[k] = complex{y.real(), 0.0};
        }
    }

    return n;
}

complex HODispersion::n_value(double lambda) const {
    return n_value_deriv(lambda, nullptr);
}

int HODispersion::fp_number() const {
    return m_oscillators.size() * OSC_PARAMETERS_NUM;
}


int HODispersion::write(Writer& w) const {
    w.printf("ho \"%s\" ", m_name.text());
    w << m_oscillators;
    return 0;
}

Writer& operator<<(Writer& w, const HODispersion::Oscillator& osc) {
    w.printf("%g %g %g %g %g", osc.nosc, osc.en, osc.eg, osc.nu, osc.phi);
    return w;
}
