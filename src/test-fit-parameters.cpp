#include "fit-params.h"

int main() {
	Strategy strategy;
	FitParameter fp(2);
	strategy.parameters.push_back(fp);
	strategy.seeds.push_back(Seed(4.5, 0.5));
	return 0;
}
