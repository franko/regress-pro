
/* disp-ho.h
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

#include "defs.h"
#include "dispers-classes.h"

__BEGIN_DECLS

struct ho_params {
    double nosc;
    double en;
    double eg;
    double nu;
    double phi;
};

struct disp_ho {
    double eps_inf;
    double eps_host;
    double nu_host;
    int nb_hos;
    struct ho_params *params;
};

struct disp_struct;

/* HO dispersion class */
extern struct disp_class ho_disp_class;

extern struct disp_struct * disp_new_ho(const char *name, int nb_hos,
                                        struct ho_params *params);
extern void disp_add_ho(struct disp_struct *d);
extern void disp_delete_ho(struct disp_struct *d, int index);
extern int disp_ho_oscillator_parameters_number(struct disp_struct *d);

__END_DECLS

#endif
