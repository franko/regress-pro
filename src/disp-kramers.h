
/* disp-kramers.h
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

#ifndef DISP_KRAMERS_H
#define DISP_KRAMERS_H

#include "defs.h"
#include "dispers-classes.h"

struct kramers_params {
    double a;
    double en;
    double eg;
    double phi;
};

struct disp_kramers {
    int n;
    kramers_params *oscillators;
};

struct disp_struct;

/* Kramers dispersion class */
extern struct disp_class kramers_disp_class;

extern void disp_add_osc(struct disp_struct *d);
extern void disp_delete_osc(struct disp_struct *d, int index);
extern int disp_kramers_oscillator_parameters_number(struct disp_struct *d);

#endif
