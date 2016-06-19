#include "str_cpp.h"
#include "str-util.h"
#include "fit-params.h"

int main() {
	Strategy strategy;
	FitParameter fp(2);
	strategy.parameters.push_back(fp);
    Seed s(4.5, 0.5);
    Writer w;
    s.write(w);
    w.save_tofile("fit-output.txt");

    str file_text;
    str_loadfile("fit-output.txt", &file_text);
    Lexer lexer(file_text.text());
    Seed s_new = Seed::read(lexer);
	strategy.seeds.push_back(s_new);
	return 0;
}
