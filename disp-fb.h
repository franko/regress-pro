
/* disp-fb.h
 *
 * Headers for Fourouhi Bloomer dispersion model.
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

__BEGIN_DECLS

struct fb_osc {
    double eg;
    double a;
    double b;
    double c;
};

struct disp_fb {
    int n;
    double n_inf;
    struct fb_osc *osc;
};

struct disp_struct;

/* FB dispersion class */
extern struct disp_class fb_disp_class;

extern struct disp_struct * disp_new_fb(const char *name, int nb_osc, double n_inf, struct fb_osc *osc);
extern void disp_add_osc(struct disp_struct *d);
extern void disp_delete_osc(struct disp_struct *d, int index);

__END_DECLS

#endif
