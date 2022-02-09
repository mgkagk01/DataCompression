#include<iostream>
#include "ArithmeticCode.h"


int main() {

	ArithmeticCode code = ArithmeticCode();
	code.encode();
	std::cout << "Rate (Average Length) = " << code.lenBits / (1.0 * 277005) << std::endl;
	code.decoder();
	
	return 0;
}