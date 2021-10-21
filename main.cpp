#include <string>
#include <iostream>
#include "dpll.hpp"

int main(int argc, char *argv[]) {
	std::string file_name = argv[1];
	DPLL dpll;
	dpll.readDIMACSFile(file_name);
	bool res = dpll.run();
	if (res)
		std::cout << "formula is SAT" << std::endl;
	else
		std::cout << "formula is UNSAT" << std::endl;
}
