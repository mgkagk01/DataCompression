// ================= The implementation has not finished yet. However, it works for the parameters below.
// The script outputs a file, called noiseRealizations.txt, that contains the realization of the noise
#include <iostream>
#include <random>
#include <math.h>
#include "TCQuantizer.h"
int main() {

	// ==================================== Parameters of the Quantizer ==================================== //
	int nStates = 4; // number of trellis states
	int R = 2; // rate
	//int nElements = pow(2,R-1); // number of elements in each sub quantizer

	std::default_random_engine generator(0);
	// ===================================== Create data to compresse ===================================== //
	std::string distributionType("uniform");
	double source[T];
	if (distributionType.compare("uniform") == 0) {
		std::uniform_real_distribution<double> distribution(-sqrt(12.0) / 2, sqrt(12.0)/ 2);
		for (int i = 0; i < T; i++)
			source[i] = distribution(generator);
	}
	else {
		std::normal_distribution<double> distribution(0.0, 1.0);
		for (int i = 0; i < T; i++)
			source[i] = distribution(generator);
	}
	

	// ===================================== Create a Quantizer ===================================== //
	TCQuantizer quantizer(nStates, R, distributionType);

	// ===================================== Compress ===================================== //
	std::vector<int> compressedData = quantizer.encode(source);

	// ===================================== Uncompress ===================================== //
	std::vector<double> dataHat = quantizer.decode(compressedData);

	return 0;
}
