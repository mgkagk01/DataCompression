#pragma once
#include "TrellisGraph.h"
#include <iostream>
#include <math.h>
#include <utility>
#include <queue>
#include <fstream>
#include <iterator>


const int nPartitions = 4;
const int nElements = 2;
const int T = 10000;
typedef struct vertex {
	int stateID;
	int quantizerID;
	int centroidID;
	int input;
	double pathMetric;
	double value2Compr;
	double noise;
	vertex* parent;
	vertex* left;
	vertex* right;

}viterbiNode;



class TCQuantizer{
	//int nElements;
public:
	// Constructor
	TCQuantizer(int nStates, int R, std::string distribution);

	// Partitions
	double centroids[nElements][nPartitions]; 
	//std::vector<std::vector<double>> centroids;
	//std::vector<double> centroids;
	TrellisGraph *graph;

	// Quantizer
	double lowerValue, highValue;
	double step;
	double intervalLen;
	int rate;
	void computeCentroids();

	// Encode
	std::vector<int> encode(double source[]);
	viterbiNode* createNode(int stateID, int quantizerID, double value2Compr, int input);
	std::vector<std::pair<int, std::pair<int, int>>> findActiveQuantizers(trellisNode* node);
	std::vector<std::pair<int, std::pair<double, int>>> createQuantizer(std::vector<std::pair<int, std::pair<int, int>>> activeQ, double value2Compr);
	viterbiNode* findMin(viterbiNode * leafs[]);
	std::vector<int> outputBits(viterbiNode* bespath);
	std::vector<double> outputCentroids(viterbiNode* bestPath);
	std::vector<double> finalCentroids;
	std::vector<int> compressedData;

	// === Decoder
	std::vector<double> decode(std::vector<int> outputBits);

	
	// === Parities (Rate 1/2 convolutional code)
	int memory; // Number of registers
	int nBranches = 2; // Number of output bits
	int h0[9]; 
	int h1[9];
	int h1Values[7] = {5, 11, 19, 37, 67, 157, 330};
	int h0Values[7] = {2, 4, 4, 8, 20, 86, 242};
	void createPolynomials(int memory);



	// === For debuging
	void printTree(viterbiNode* root);
	void checkCodeQuantizer(std::vector<double> reconstructedValues);
	void saveNoiseRealizations(viterbiNode* bestPath);

};

