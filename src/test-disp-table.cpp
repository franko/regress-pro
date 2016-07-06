#include <stdio.h>

#include "HODispersion.h"
#include "DispersionTable.h"
#include "str-util.h"

void print_n_and_k(const Dispersion& d) {
	complex n = d.n_value(190.0);
	printf("190nm: %g %g\n", n.real(), -n.imag());
	n = d.n_value(250.0);
	printf("250nm: %g %g\n", n.real(), -n.imag());
	n = d.n_value(633.0);
	printf("633nm: %g %g\n", n.real(), -n.imag());
}

int main() {
	Dispersion::register_class(ho_dispersion_class);
	Dispersion::register_class(dispersion_table_class);

	auto disp = DispersionTable::load_mat_file("../examples/Jellison-Silicon.mat");

	Writer w;
	disp->write(w);
	w.save_tofile("output-disp-table.txt");

	print_n_and_k(*disp);

	return 0;
}
