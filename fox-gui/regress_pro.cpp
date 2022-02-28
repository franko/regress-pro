#include "regress_pro.h"
#include "icons_all.h"

#ifdef WIN32
#define SCRIPT_FONT_NAME "Consolas"
#else
#define SCRIPT_FONT_NAME "Monospace"
#endif

FXDEFMAP(regress_pro) regress_pro_map[]= {
};

// Object implementation
FXIMPLEMENT(regress_pro, registered_app, regress_pro_map,ARRAYNUMBER(regress_pro_map));

regress_pro::regress_pro() :
    registered_app("Regress Pro", "Francesco Abbate"),
    appicon(this, regressproicon),
    small_font(this, "helvetica", 9, FXFont::Normal, FXFont::Straight),
    bold_font(this, "helvetica", 9, FXFont::Bold, FXFont::Italic),
    monospace_font(this, SCRIPT_FONT_NAME, 9),
    big_web_font(this, "helvetica", 14, FXFont::Bold, FXFont::Straight),
    blue_web(FXRGB(100,114,161)), blue_highlight(FXRGB(3,12,180)), red_warning(FXRGB(180,5,10)), black(FXRGB(0,0,0))
{
    delete_icon = new FXGIFIcon(this, delete_gif);
    add_icon = new FXGIFIcon(this, new_gif);
}

double regress_pro::scale() const {
    const double scale_factor = (double) monospace_font.getCharWidth('x') / 6.66667;
    if (scale_factor <= 1.01) {
        return 1.0;
    }
    return scale_factor;
}

regress_pro::~regress_pro()
{
	delete delete_icon;
	delete add_icon;
}

