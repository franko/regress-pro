#include <stdio.h>

#include "HODispersion.h"
#include "str-util.h"

void print_n_and_k(const Dispersion& d) {
	printf("%s\n", d.name().text());
	complex n = d.n_value(190.0);
	printf("190nm: %g %g\n", n.real(), -n.imag());
	n = d.n_value(250.0);
	printf("250nm: %g %g\n", n.real(), -n.imag());
	n = d.n_value(633.0);
	printf("633nm: %g %g\n", n.real(), -n.imag());
}

int main() {
	Dispersion::register_class(ho_dispersion_class);

	HODispersion d{"HO test (\"with quotes\")"};

	d.add_oscillator(HODispersion::Oscillator{150.0, 15.7, 0.0, 0.3333,  0.0});
	d.add_oscillator(HODispersion::Oscillator{10.0,  6.5,  0.3, 0.3333, -0.7});

	print_n_and_k(d);

	Writer w;
	d.write(w);
	w.save_tofile("ho-output.txt");

	str text;
	str_loadfile("ho-output.txt", &text);
	Lexer lexer(text.text());

	auto d_new = Dispersion::read(lexer);
	print_n_and_k(*d_new);

	HODispersion d_ter(d);
	print_n_and_k(d_ter);

	return 0;
}
