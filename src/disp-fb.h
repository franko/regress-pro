
/* disp-fb.h
 *
 * Headers for Forouhi-Bloomer dispersion model.
 *
 * Copyright (C) 2005-2015 Francesco Abbate
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

#ifndef DISP_FB_H
#define DISP_FB_H

#include "defs.h"
#include "dispers-classes.h"

/* Can use the original FB parameters or an alternative form where:
   A = A' C'^2
   B = 2 B'
   C = C'^2 + B'^2
   where A, B, C are the original FB parameter and A', B' and C' are the
   redefined parameters used in the struct below.
   The redefined parameter are related to the Lorentzian peak term in a simpler way.
   The redefined parameter B' is the approx position of the peak, in eV.
   A' is the height of the peak and C' is its width. */
struct fb_osc {
    double a;
    double b;
    double c;
};

enum {
  FOROUHI_BLOOMER_STANDARD = 0,
  FOROUHI_BLOOMER_RATIONAL = 1,
};

enum {
    TAUC_LORENTZ_STANDARD = FOROUHI_BLOOMER_STANDARD,
    TAUC_LORENTZ_RATIONAL = FOROUHI_BLOOMER_RATIONAL,
};

struct disp_fb {
    short int n; /* Number of oscillator. */
    short int form; /* Indicate the form of coefficients. */
    double n_inf;
    double eg;
    struct fb_osc *osc;
};

struct disp_struct;

/* Forouhi-Bloomer dispersion class. */
extern struct disp_class fb_disp_class;

/* Tauc-Lorentz dispersion class. */
extern struct disp_class tauc_lorentz_disp_class;

extern struct disp_struct * disp_new_fb(const char *name, int coeff_form, int nb_osc, double n_inf, double eg, struct fb_osc *osc);
extern struct disp_struct * disp_new_tauc_lorentz(const char *name, int coeff_form, int nb_osc, double n_inf, double eg, struct fb_osc *osc);
extern void disp_fb_change_form(struct disp_struct *d, int new_coeff_form);
extern void disp_add_osc(struct disp_struct *d);
extern void disp_delete_osc(struct disp_struct *d, int index);

#endif
