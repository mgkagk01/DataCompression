#include "GolmobCode.h"


GolmobCode::GolmobCode(int const n, float p, int k = NULL) {

	lenSource = n;
	// GolmobCode is optimal when the probability is less than 0.5
	// Encode the symbol using Unary Code or equivalently Comma Code
	if (p < 0.5)
		k = 0;

	// If the user did define the parameter k, compute it
	else if (k == NULL && p > 0.5) {
		// Compute the best m
		double temp = (1.0 * log(0.5)) / (1.0 * log(p));
		k = ceil(log(temp) / log(2));
	}

	m = pow(2, k);
	lenXr = k;


}

void GolmobCode::encoder(int source[]) {


	// Open file to store the integers
	std::ofstream intFile;
	intFile.open("integersFile.txt");
	// Open file to store the binary codewords
	std::ofstream binFile;
	binFile.open("binFile.dat", std::ios::out | std::ios::binary);

	int xq, xr;
	int lastIdx = -1;
	// Compute the quotient and reminder for each symbol
	for (int i = 0; i < lenSource; i++){

		// Save to file
		intFile << source[i];

		

		std::cout << "Symbol to encode: " << source[i] << std::endl;
		// === Compute quotient
		xq = source[i] / m;

		// Encode the quotient
		for (int j = lastIdx + 1; j < lastIdx + 1 + xq; j++){
			compressedData.push_back(0);
			//binFile.write((bool*)&0, sizeof(bool));
		}

		lastIdx += xq + 1;
		compressedData.push_back(1);
		binFile << 1;
	

		// === Compute reminder 
		xr = source[i] % m;

		// === Encode xr
		// Find the binary representation of xr
		compressedData.push_back(0);
		compressedData.push_back(0);
		for (int j = lastIdx + lenXr; j > lastIdx ; j--){
			compressedData[j] = xr % 2;
			xr = xr / 2;
			binFile << 0;
		}
		lastIdx += lenXr;
		


	}
	lastIdx++;
	intFile.close();
	binFile.close();
	printCompressed(compressedData, lastIdx);


}
//void GolmobCode::decoder(bool compressed[], int size){
//
//	int source[10]
//
//
//}