#include<iostream>
#include<random>
#include "GolmobCode.h"
#include "utility.h"
int main() {

    // Source
    float p = 0.75;
    float prob = 1 - p;
    int const lenSource = 10;
    std::default_random_engine generator(0);
    std::geometric_distribution<int> distribution(prob);

    int source[lenSource] = {};

    // Generate data
    for (int i = 0; i < lenSource; i++) {
        source[i] = distribution(generator);
    }

    // Print source
    printData(source, lenSource);

    // ===== Golmob Code
   GolmobCode code(lenSource, p, NULL);

   code.encoder(source);
	return 0;
}