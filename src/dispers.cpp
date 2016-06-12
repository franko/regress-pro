#include "dispers.h"

eastl::vector<DispersionClass> Dispersion::registered_classes;

const DispersionClass *lookup_dispersion_class(const char *id) {
    for (const DispersionClass& klass : Dispersion::registered_classes) {
        if (klass.short_name == id) {
            return &klass;
        }
    }
    return nullptr;
}

std::unique_ptr<Dispersion> Dispersion::read(Lexer& lexer) {
#if 0
    if (lexer_check_ident(l, "library") == 0) {
        if (lexer_string(l)) return NULL;
        return lib_disp_table_get(CSTR(l->store));
    }
#endif
    const char *id = lexer.lookup_ident();
    if (!id) return nullptr;
    const DispersionClass *klass = lookup_dispersion_class(id);
    if (!klass) return nullptr;
    return klass->read(lexer);
}
