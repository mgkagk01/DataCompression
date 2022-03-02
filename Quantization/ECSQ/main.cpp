#include<iostream>
#include "ArithmeticCode.h"
#include <random>
#include <vector>
#include "ECSQ.h"
int main() {


	// ======================= Source ======================= //
	std::default_random_engine generator;
	std::normal_distribution<double> distribution(0, 1.0);

	// Generate Gaussian random data
	double data[dataSize];
	for (int i = 0; i < dataSize; ++i)
		data[i] = distribution(generator);


	// ======================= Quantizer ======================= //
	int R = 4; 

	ECSQ quantizer(R);

	quantizer.encode(data);

	std::vector<double> recovaredValues = quantizer.decode();

	// ======================= Check that ECSQ works ======================= //
	for (int i = 0; i < dataSize; i++) {
		if (std::abs(recovaredValues[i] - quantizer.centroids[quantizer.indices[i]]) > 0.01) {
			std::cout << recovaredValues[i] <<  std::endl;
			std::cout << quantizer.centroids[quantizer.indices[i]] << std::endl;
			std::cout << "Problem";
		}
	}

	return 0;
}


