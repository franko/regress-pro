#include "dispers_win.h"
#include "sampling_win.h"

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
      m_dispers(disp), m_sampling(get_disp_wavelength_sampling(disp)),
      m_plot_n{libcanvas::Plot::AutoLimits|libcanvas::Plot::ShowUnits}, m_plot_k{libcanvas::Plot::AutoLimits|libcanvas::Plot::ShowUnits}
{

    FXMenuBar *menubar = new FXMenuBar(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X);

    // Dispersion menu
    dispmenu = new FXMenuPane(this);
    new FXMenuCommand(dispmenu,"Spectral &Range",nullptr,this,ID_SPECTR_RANGE);
    new FXMenuCommand(dispmenu,"&Close",nullptr,this,ID_CANCEL);
    new FXMenuTitle(menubar,"&Dispersion",nullptr,dispmenu);

    FXVerticalFrame *topfr = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    m_canvas = new FXLibcanvasWindow(topfr, "v..", LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXHorizontalSeparator(topfr,SEPARATOR_GROOVE|LAYOUT_FILL_X);
    FXHorizontalFrame *buttonfr = new FXHorizontalFrame(topfr,LAYOUT_FILL_X|LAYOUT_RIGHT);
    new FXButton(buttonfr,"&Close",nullptr,this,ID_CANCEL,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);

    config_plot();
}

dispers_win::~dispers_win() {
    delete dispmenu;
};

static void add_nk_plots_with_sampling(libcanvas::Plot& plot_n, libcanvas::Plot& plot_k, const disp_t *dispers, const sampling_unif& sampling) {
    libcanvas::Path n_path, k_path;
    for (unsigned k = 0; k < sampling.size(); k++) {
        const double wavelength = sampling[k];
        cmpl n = n_value(dispers, wavelength);
        n_path.LineTo(wavelength,  std::real(n));
        k_path.LineTo(wavelength, -std::imag(n));
    }
    plot_n.AddStroke(std::move(n_path), libcanvas::color::Red, 1.5);
    plot_k.AddStroke(std::move(k_path), libcanvas::color::Red, 1.5);
}

void
dispers_win::config_plot()
{
    m_plot_n.SetClipMode(false);
    m_plot_k.SetClipMode(false);
    add_nk_plots_with_sampling(m_plot_n, m_plot_k, m_dispers, m_sampling);
    m_canvas->Attach(m_plot_n, "2");
    m_canvas->Attach(m_plot_k, "1");
}

long
dispers_win::on_cmd_set_range(FXObject*,FXSelector,void*)
{
    sampling_win win(this, &m_sampling);
    if(win.execute()) {
        m_plot_n.ClearLayer();
        m_plot_k.ClearLayer();
        add_nk_plots_with_sampling(m_plot_n, m_plot_k, m_dispers, m_sampling);
        return 1;
    }
    return 0;
}
