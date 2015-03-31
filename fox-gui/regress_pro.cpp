#include "regress_pro.h"
#include "icons_all.h"

FXDEFMAP(regress_pro) regress_pro_map[]= {
};

// Object implementation
FXIMPLEMENT(regress_pro, registered_app, regress_pro_map,ARRAYNUMBER(regress_pro_map));

regress_pro::regress_pro() :
    registered_app("Regress Pro", "Francesco Abbate"),
    appicon(this, regressproicon)
{
}
