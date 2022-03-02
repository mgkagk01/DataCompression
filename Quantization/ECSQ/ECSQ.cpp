#include "ECSQ.h"
ECSQ::ECSQ(int rate) {

	R = rate;
	// Number of cells
	nCells = pow(2, R);

	// Step size
	step = interval / nCells;

	boundaries.resize(nCells + 1,0);

	// Compute boundaries
	boundaries[0] = lower;
	boundaries[nCells] = upper;
	for (int i = 1; i < nCells; i++)
		boundaries[i] = boundaries[i - 1] + step;

	// Compute centroids
	centroids.resize(nCells, 0);
	for (int i = 1; i < nCells + 1; i++)
		centroids[i - 1] = (boundaries[i] + boundaries[i - 1]) / 2.0;

	// Recovered Values
	recoveredValues.resize(dataSize, 0);


}



void ECSQ::encode(double data[]) {
	// Distortion
	mse = 0;

	// First step - Scalar Uniform Quantization
	uniformQuant(data);

	// Second step - Entropy Coding (Arithmetic Encoding)
	arithmeticEn();

}

void ECSQ::uniformQuant(double data[]) {
	for (int i = 0; i < dataSize; i++) {
		distance = 100;
		if (data[i] > 0) {
			// Go right
			for (int j = nCells / 2; j < nCells; j++) {
				if (distance > pow(data[i] - centroids[j], 2)) {
					distance = pow(data[i] - centroids[j], 2);
					indices[i] = j;
				}

			}
		}
		else {
			
			// Go left
			for (int j = nCells / 2; j >= 0; j--) {
				if (distance > pow(data[i] - centroids[j], 2)) {
					distance = pow(data[i] - centroids[j], 2);
					indices[i] = j;
				}
			}
		}

		mse += pow(data[i] - centroids[indices[i]], 2);
	}

	std::cout<< "MSE = " << mse / dataSize << std::endl;
	double snr = 1/ (mse / dataSize);
	std::cout << "SNR(dB) = " << 10 * (log(snr) / log(10)) << "dB" << std::endl;
}


void ECSQ::arithmeticEn() {
	arith = new ArithmeticCode(nCells, dataSize);

	arith->encode(indices);
}

std::vector<double> ECSQ::decode(){

	// First step -  Entropy Coding (Arithmetic Decoding)
	std::vector<int> indicesHat = arith->decoder();

	
	// Second step - From Indices to Centroids
	for (int i = 0; i < dataSize; i++){
		recoveredValues[i] = centroids[indicesHat[i]];
	}


	return recoveredValues;
}