
/* disp-lorentz.h
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

#ifndef DISP_LORENTZ_H
#define DISP_LORENTZ_H

#include "defs.h"
#include "dispers-classes.h"

__BEGIN_DECLS

struct lorentz_osc {
    double a;
    double en;
    double br;
};

struct disp_lorentz {
    int oscillators_number;
    double e_offset;
    struct lorentz_osc *oscillators;
};

struct disp_struct;

/* Lorentz dispersion class */
extern struct disp_class lorentz_disp_class;

extern struct disp_struct * disp_new_lorentz(const char *name, int oscillators_no, struct lorentz_osc *params);
extern void disp_lorentz_add_oscillator(struct disp_struct *d);
extern void disp_lorentz_delete_oscillator(struct disp_struct *d, int index);
extern int disp_lorentz_oscillator_parameters_number(struct disp_struct *d);

__END_DECLS

#endif
