#include "utility.h"
void printData(int source[], int const lenSource) {

	std::cout << "Print Source\n";
	for (int i = 0; i < lenSource; i++){
		std::cout << source[i] << " ";
	}
	std::cout << std::endl;
}

void printCompressed(std::vector<bool> compressed, int const lenSource) {

	std::cout << "Print compressed\n";
	for (int i = 0; i < lenSource; i++) {
		std::cout << compressed[i] << " ";
	}
	std::cout << std::endl;
}


void dec2bin(int decNum, int binArray[], int size) {


	// counter for binary array
	int numBits = ceil(log(decNum) / log(2));
	for (int i = 0; i < size-numBits; i++){
		binArray[i] = 0;
	}


	int i = size -1;
	while (decNum > 0) {
		// storing remainder in binary array
		binArray[i] = decNum % 2;
		decNum = decNum / 2;
		i--;
	}

}



int bin2dec(int binArray[], int size) {

	int decNum = 0;

	for (int i = 0; i < size; i++){
		decNum++;
	}

	return decNum;
}