#include "dispers.h"

eastl::vector<const DispersionClass *> Dispersion::m_registered_classes;

void Dispersion::register_class(const DispersionClass& d) {
    m_registered_classes.push_back(&d);
}

const DispersionClass* Dispersion::lookup_class(const char *short_name) {
    for (const DispersionClass* klass : m_registered_classes) {
        if (klass->short_name == short_name) {
            return klass;
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
    if (!id) throw std::invalid_argument("expect identifier");
    const DispersionClass *klass = lookup_class(id);
    if (!klass) throw std::invalid_argument("unknown dispersion type");
    return klass->read(lexer);
}
