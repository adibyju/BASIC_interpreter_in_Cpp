#include <iostream>
#include <string>
#include "basic.h"

int main() {
	std::string inp, final_inp="";
	while (true) {
		std::cout << "basic > ";
		std::getline(std::cin, inp);

		final_inp = "";
		for (auto x : inp) {
			if (x != ' ' && x != '\t') final_inp += x;
		}

		if (final_inp.empty()) continue;
		
		std::pair<std::shared_ptr<Node>, Error> finalResult = run("<stdin>", final_inp);

		std::shared_ptr<Node> ast = finalResult.first;
		Error error = finalResult.second;

		if (error.is_error() != "None" && error.is_error() != "Runtime Error: Division by zero") {
			std::cout<<"Entered error block\n";
			std::cout << error.as_string() << std::endl;
		}
		else if (error.is_error() == "Runtime Error: Division by zero") {
			continue;
		}
		else {
			//std::cout << *ast << std::endl;
		}
		std::cout << std::endl;
	}
	return 0;
}