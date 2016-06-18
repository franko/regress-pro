#include "fit-params.h"

unique_ptr<Seed> Seed::read(Lexer& lexer) {
    if (lexer.ident_to_store()) return nullptr;
    if (lexer.store == "value") {
        double value;
        lexer.number(&value);
        return unique_ptr<Seed>(new Seed(value));
    } else if (lexer.store == "range") {
        double value, delta;
        lexer.number(&value);
        lexer.number(&delta);
        return unique_ptr<Seed>(new Seed(value, delta));
    } else if (lexer.store == "undef") {
        return unique_ptr<Seed>(new Seed());
    }
    return nullptr;
}
