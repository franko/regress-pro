
#include "dispers_win.h"
#include "sampling_win.h"
#include "disp_vs.h"

static sampling_unif get_disp_wavelength_sampling(disp_t* disp) {
    double wavelength_start, wavelength_end;
    int samples_number;
    disp_get_wavelength_range(disp, &wavelength_start, &wavelength_end, &samples_number);
    return sampling_unif(wavelength_start, wavelength_end, samples_number);
}

// Map
FXDEFMAP(dispers_win) dispers_win_map[]= {
    FXMAPFUNC(SEL_COMMAND, dispers_win::ID_SPECTR_RANGE, dispers_win::on_cmd_set_range),
};

// Object implementation
FXIMPLEMENT(dispers_win,FXDialogBox,dispers_win_map,ARRAYNUMBER(dispers_win_map));

dispers_win::dispers_win(FXWindow* w, disp_t* disp)
    : FXDialogBox(w, "Dispersion Plot", DECOR_ALL, 0, 0, 480, 360, 0,0,0,0,0,0),
      m_dispers(disp), m_sampling(get_disp_wavelength_sampling(disp))
{

    FXMenuBar *menubar = new FXMenuBar(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X);

    // Dispersion menu
    dispmenu = new FXMenuPane(this);
    new FXMenuCommand(dispmenu,"Spectral &Range",NULL,this,ID_SPECTR_RANGE);
    new FXMenuCommand(dispmenu,"&Close",NULL,this,ID_CANCEL);
    new FXMenuTitle(menubar,"&Dispersion",NULL,dispmenu);

    FXVerticalFrame *topfr = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    m_canvas = new plot_canvas(topfr, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXHorizontalSeparator(topfr,SEPARATOR_GROOVE|LAYOUT_FILL_X);
    FXHorizontalFrame *buttonfr = new FXHorizontalFrame(topfr,LAYOUT_FILL_X|LAYOUT_RIGHT);
    new FXButton(buttonfr,"&Close",NULL,this,ID_CANCEL,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);

    config_plot();
}

void
dispers_win::config_plot()
{
    disp_t* d = m_dispers;
    sampling_unif& samp = m_sampling;

    disp_vs<sampling_unif>* disp_n = new disp_vs<sampling_unif>(d, cmpl::real_part, samp);
    add_new_simple_plot(m_canvas, disp_n, "refractive index");

    disp_vs<sampling_unif>* disp_k = new disp_vs<sampling_unif>(d, cmpl::imag_part, samp);
    add_new_simple_plot(m_canvas, disp_k, "absoption coeff");

    m_canvas->set_dirty(true);
}

long
dispers_win::on_cmd_set_range(FXObject*,FXSelector,void*)
{
    sampling_win win(this, &m_sampling);
    if(win.execute()) {
        m_canvas->update_limits();
        m_canvas->set_dirty(true);
        return 1;
    }
    return 0;
}
