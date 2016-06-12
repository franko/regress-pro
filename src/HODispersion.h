
/* HODispersion.h
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

#ifndef DISP_HO_H
#define DISP_HO_H

#include <memory>
#include <EASTL/vector.h>

#include "defs.h"
#include "dispers.h"

struct HODispersionClass : DispersionClass {
	using DispersionClass::DispersionClass;
	std::unique_ptr<Dispersion> read(Lexer& lexer) override;
};

class HODispersion : public Dispersion {
public:
	struct Oscillator {
		double nosc, en, eg, nu, phi;
	};

	using size_type = eastl::vector<Oscillator>::size_type;

	HODispersion(const char *name): Dispersion(name, m_klass)
	{ }

	HODispersion(const char *name, int size): Dispersion(name, m_klass), m_oscillators(size_type(size))
	{ }

	const Oscillator& oscillator(int i) const { return m_oscillators[i]; }
	      Oscillator& oscillator(int i)       { return m_oscillators[i]; }

	void add_oscillator(const Oscillator& osc) {
		m_oscillators.push_back(osc);
	}

	complex n_value(double lambda) const override;
    complex n_value_deriv(double lam, complex der[]) const override;
    int fp_number() const override;
    int write(Writer& w) const override;
    // static std::unique_ptr<HODispersion> read(const char *name, Lexer& lexer);


private:
	enum { NOSC = 0, EN = 1, EG = 2, NU = 3, PHI = 4 };
	enum { OSC_PARAMETERS_NUM = 5 };

	int parameter_index(int index_osc, int index_param) const {
		return index_osc * OSC_PARAMETERS_NUM + index_param;
	}

	static const HODispersionClass m_klass;
	static const char *parameter_names[];

	eastl::vector<Oscillator> m_oscillators;
};

Writer& operator<<(Writer& w, const HODispersion::Oscillator& osc);

#endif
