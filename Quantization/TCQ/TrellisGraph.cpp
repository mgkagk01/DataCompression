#include "TrellisGraph.h"


TrellisGraph::TrellisGraph(int h0[], int h1[], int memory) {

	//nStates = numStates;
	//adj = new trellisNode[numStates];
	std::vector<int> registers;
	for (int i = 0; i < memory; i++)
		registers.push_back(-1);
	
	// Create new node
	int inputBit;
	int sumh0;
	int sumh1;
	int output;
	int decNum;
	int count;
	int nextNodeID;
	// For each state
	for (int i = 0; i < nStates; i++){


		// Find the values in the registers
		decNum = i;
		count = memory - 1;
		while (decNum > 0) {
			// storing remainder in binary array
			registers[count] = decNum % 2;
			decNum = decNum / 2;
			count--;
		}
		// Fill with zeros
		for (int c = count; c >= 0; c--){
			registers[c] = 0;
		}

		

		sumh0 = 0;
		sumh1 = 0;
		trellisNode* temp; 


		// For each output bit
		for (int j = 0; j < memory; j++) {
			if (h0[j + 1])
				sumh0 += registers[j];
			if (h1[j + 1])
				sumh1 += registers[j];
		}

		sumh0 = sumh0 % 2;
		sumh1 = sumh1 % 2;

		// For each input bit
		for (int k = 0; k < 2; k++) {


			output = (2 * sumh1) + sumh0;
			
			registers.push_back(k);
			nextNodeID = bin2dec(registers);
			registers.pop_back();


			if (k == 0) {
				adj[i] = createNode(nextNodeID, k, output);
			}
			else {
				adj[i]->next = createNode(nextNodeID, k, output);
			}

			if (h0[0])
				sumh0 ^= 1;
			if (h1[0])
				sumh1 ^= 1;
			
		}
				
	}

}

int TrellisGraph::bin2dec(std::vector<int> vector) {
	int decNum = 0;
	//int len = pow(2, vector.size()-2);
	int j = 1;
	for (int i = vector.size() - 3; i >=0; i--){
		decNum += vector[i] * j;
		j = 2 * j;
	}
	decNum += vector.back() * j;

	return decNum;

}

trellisNode* TrellisGraph::createNode(int nextNode, int input, int output) {

	trellisNode* newNode = new trellisNode();
	newNode->nextNode = nextNode;
	newNode->input = input;
	newNode->output = output;
	newNode->next = NULL;
	return newNode;
}