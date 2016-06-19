#include "fit-params.h"

Seed Seed::read(Lexer& lexer) {
    str tag;
    lexer >> tag;
    if (tag == "value") {
        double seed;
        lexer >> seed;
        return Seed(seed);
    } else if (tag == "range") {
        double seed, delta;
        lexer >> seed >> delta;
        return Seed(seed, delta);
    } else if (tag == "undef") {
        return Seed();
    }
    throw std::invalid_argument("expect seed specification");
}
