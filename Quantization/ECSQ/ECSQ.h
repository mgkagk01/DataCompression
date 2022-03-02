#pragma once
#include <math.h>
#include<iostream>
#include <vector>
#include "ArithmeticCode.h"
const int dataSize = 80000;
class ECSQ
{
	public:
		ECSQ(int R);
		// ======================= Quantizer ======================= //
		int R; // Rate

		// Upper and lower bound
		double lower = -3;
		double upper = 3;
		double interval = upper - lower;

		// Step size
		double step;

		// Number of cells
		int nCells;

		// boundaries
		std::vector<double> boundaries;

		// centroids
		std::vector<double> centroids;

		// To store the indices
		int indices[dataSize];
		double distance;

		void uniformQuant(double data[]);
		double mse;
		std::vector<double> recoveredValues;
		

		// ======================= Entropy Coding ======================= //
		
		ArithmeticCode *arith;

		// Arithmetic coding
		void arithmeticEn();

		// Encoder
		void encode(double data[]);

		std::vector<double> decode();
};
	

