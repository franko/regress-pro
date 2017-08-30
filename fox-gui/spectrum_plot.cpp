#include <assert.h>

#include "spectrum_plot.h"
#include "spectrum_vs.h"
#include "elliss.h"

void
spectra_plot_simple(plot_canvas* canvas, struct spectrum* spectr)
{
    enum system_kind skind = spectr->acquisition->type;

    canvas->clear_plots();

    switch(skind) {
    case SYSTEM_SR: {
        spectrum_vs *ref_r = new spectrum_vs(spectr);
        add_new_simple_plot(canvas, ref_r, "reflectance");
        break;
    }
    case SYSTEM_SE_RPE:
    case SYSTEM_SE_RAE:
    case SYSTEM_SE: {
        const int se_type = SE_TYPE(skind);

        char const * const title0 = (se_type == SE_ALPHA_BETA ? "SE alpha" : "SE tan(psi)");
        spectrum_vs *ref_c0 = new spectrum_vs(spectr, 0);
        add_new_simple_plot(canvas, ref_c0, title0);

        char const * const title1 = (se_type == SE_ALPHA_BETA ? "SE beta" : "SE cos(delta)");
        spectrum_vs *ref_c1 = new spectrum_vs(spectr, 1);
        add_new_simple_plot(canvas, ref_c1, title1);

        break;
    }
    default:
        /* */
        ;
    }
}

void
spectra_plot(plot_canvas* canvas, struct spectrum* ref_spectr,
             struct spectrum *mod_spectr)
{
    enum system_kind skind = ref_spectr->acquisition->type;

    assert(ref_spectr->acquisition->type == mod_spectr->acquisition->type);

    canvas->clear_plots();

    switch(skind) {
    case SYSTEM_SR: {
        spectrum_vs *ref_r = new spectrum_vs(ref_spectr);
        spectrum_vs *mod_r = new spectrum_vs(mod_spectr);
        add_new_plot(canvas, ref_r, mod_r, "reflectance");
        break;
    }
    case SYSTEM_SE_RPE:
    case SYSTEM_SE_RAE:
    case SYSTEM_SE: {
        const int se_type = SE_TYPE(skind);

        char const * const title0 = (se_type == SE_ALPHA_BETA ? "SE alpha" : "SE tan(psi)");
        spectrum_vs *ref_c0 = new spectrum_vs(ref_spectr, 0);
        spectrum_vs *mod_c0 = new spectrum_vs(mod_spectr, 0);
        add_new_plot(canvas, ref_c0, mod_c0, title0);

        char const * const title1 = (se_type == SE_ALPHA_BETA ? "SE beta" : "SE cos(delta)");
        spectrum_vs *ref_c1 = new spectrum_vs(ref_spectr, 1);
        spectrum_vs *mod_c1 = new spectrum_vs(mod_spectr, 1);
        add_new_plot(canvas, ref_c1, mod_c1, title1);

        break;
    }
    default:
        /* */
        ;
    }
}
