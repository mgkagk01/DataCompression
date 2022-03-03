#include "TCQuantizer.h"


TCQuantizer::TCQuantizer(int nStates, int R, std::string distribution) {
	
	//nElements = pow(2, R - 1);
	//centroids.resize(nElements, std::vector<double>(nPartitions));
	//centroids.resize(nElements,nPartitions);
	// Create the polynomials for the convolutional coding
	createPolynomials(nStates);

	// Create the Trellis graph
	graph = new TrellisGraph(h0, h1, memory);

	// Initialize the Scalar Quantizer
	rate = R;
	if (distribution.compare("uniform") == 0) {
		lowerValue = -sqrt(12.0) / 2;
		highValue = sqrt(12.0) / 2;
	}
	else {
		lowerValue = -3;
		highValue = 3;
	}
	
	computeCentroids();


}


void TCQuantizer::computeCentroids() {
	double bandwidth = (highValue - lowerValue);
	intervalLen = bandwidth / pow(2, rate + 1);
	double previousValue = lowerValue;
	double nextValue = lowerValue + intervalLen;
	
	for (int i = 0; i < nElements; i++){
		for (int j = 0; j < nPartitions; j++){
			centroids[i][j] = (previousValue + nextValue) / 2;
			previousValue = nextValue;
			nextValue = nextValue + intervalLen;
		}
	}



}

std::vector<int> TCQuantizer::encode(double source[]) {

	// Create the root
	viterbiNode* root = createNode(0, 0, source[0],0);

	
	viterbiNode* activeEven[nStates];
	viterbiNode* activeOdd[nStates];
	for (int i = 0; i < nStates; i++) {
		activeEven[i] = NULL;
		activeOdd[i] = NULL;
	}
		
	activeEven[0] = root;

	// Find which of the quantizers are available
	std::vector<std::pair<int, std::pair<int,int>>> activeQ;

	// Create a Quanizer
	std::vector<std::pair<int, std::pair<double, int>>> quantizer;
	double distortion;
	int nextState;
	viterbiNode* newNode;
	// For all times
	for (int t = 0; t < T; t++){

		// ===================================================== Process Even levels ===================================================== //
		if (t % 2 == 0){ 

			// For all active states at this level
			for (int i = 0; i < nStates; i++) {

				// Check if the node is active
				if (activeEven[i] != NULL) {
					// Find the active Quantizers
					activeQ = findActiveQuantizers(graph->adj[i]); // activeQ.first = quantizer ID, activeQ.second = next State

					// Construct the Quantizer: Based on the value of the source create the quantizer
					quantizer = createQuantizer(activeQ, source[t]); // quantizer.first = quantizer ID
																	 // quantizer.second.first = value of centroid
																	 // quantizer.second.second = centroid ID

					// For all the cells of the Quantizer
					for (int q = 0; q < quantizer.size(); q++) {
						// Compute distortion
						distortion = pow(quantizer[q].second.first - source[t], 2);

						// Check if next State for this level is occupied
						if (activeOdd[activeQ[q].second.first] == NULL) {
							// --- The state is available

							// Create a node
							newNode = createNode(activeQ[q].second.first, activeQ[q].first, source[t], activeQ[q].second.second);
							newNode->pathMetric = activeEven[i]->pathMetric + distortion; // Update the pathmetric
							newNode->parent = activeEven[i]; 
							newNode->centroidID = quantizer[q].second.second;
							newNode->noise = quantizer[q].second.first - source[t]; //New line
							
							// Add this node to the child of the parent
							if (activeEven[i]->left == NULL)
								activeEven[i]->left = newNode;
							else
								activeEven[i]->right = newNode;

							// Mark this node as the active node at this level
							activeOdd[activeQ[q].second.first] = newNode;
						}

						else {

							// Select the smallest path metric
							if (activeOdd[activeQ[q].second.first]->pathMetric > activeEven[i]->pathMetric + distortion) {
								// --- If true, change the values in this node

								// Change the path metric
								activeOdd[activeQ[q].second.first]->pathMetric = activeEven[i]->pathMetric + distortion;

								// Update the child of the parent to NULL
								if (activeOdd[activeQ[q].second.first]->parent->left == activeOdd[activeQ[q].second.first])
									activeOdd[activeQ[q].second.first]->parent->left = NULL;
								else
									activeOdd[activeQ[q].second.first]->parent->right = NULL;

								// Make the parent of the active node the node in the previous level
								activeOdd[activeQ[q].second.first]->parent = activeEven[i];

								// Update the centroid ID, state ID and quantizer ID
								activeOdd[activeQ[q].second.first]->centroidID = quantizer[q].second.second;
								activeOdd[activeQ[q].second.first]->stateID = activeQ[q].second.first;
								activeOdd[activeQ[q].second.first]->quantizerID = activeQ[q].first;

								// Update the value to be compressed
								activeOdd[activeQ[q].second.first]->value2Compr = source[t];
								activeOdd[activeQ[q].second.first]->noise = quantizer[q].second.first - source[t]; // New line

								// Add this node as a child to the parent in the previous level
								if (activeEven[i]->left == NULL)
									activeEven[i]->left = activeOdd[activeQ[q].second.first];
								else
									activeEven[i]->right = activeOdd[activeQ[q].second.first];
							}
						}
						
					}


				}


			}

			// Initialize the active Even level array to NULL for the next round
			if(t != T-1)
				for (int i = 0; i < nStates; i++)
					activeEven[i] = NULL;
		
		}

		// ===================================================== Process Odd levels ===================================================== //
		else {
			// For all active states at this level
			for (int i = 0; i < nStates; i++) {
				if (activeOdd[i] != NULL) {
					// Find the active Quantizers
					activeQ = findActiveQuantizers(graph->adj[i]); // activeQ.first = quantizer ID, activeQ.second = next State

					// Construct the Quantizer: Based on the value of the source create the quantizer
					quantizer = createQuantizer(activeQ, source[t]); // quantizer.first = quantizer ID
																	 // quantizer.second.first = value of centroid
																	 // quantizer.second.second = centroid ID
					
					// For all the cells of the Quantizer
					for (int q = 0; q < quantizer.size(); q++) {
						// Compute distortion
						distortion = pow(quantizer[q].second.first - source[t], 2);

						// Check if next State for this level is occupied
						if (activeEven[activeQ[q].second.first] == NULL) {
							// --- The state is available

							// Create a node
							newNode = createNode(activeQ[q].second.first, activeQ[q].first, source[t], activeQ[q].second.second);
							newNode->pathMetric = activeOdd[i]->pathMetric + distortion; // Update the pathmetric
							newNode->parent = activeOdd[i];
							newNode->centroidID = quantizer[q].second.second;
							newNode->noise = quantizer[q].second.first - source[t];
							// Update the child of the parent to NULL
							if (activeOdd[i]->left == NULL)
								activeOdd[i]->left = newNode;
							else
								activeOdd[i]->right = newNode;

							// Mark this node as the active node at this level
							activeEven[activeQ[q].second.first] = newNode;
						}

						else {
							// Select the smallest path metric
							if (activeEven[activeQ[q].second.first]->pathMetric > activeOdd[i]->pathMetric + distortion) {
								// --- If true, change the values in this node

								// Change the path metric
								activeEven[activeQ[q].second.first]->pathMetric = activeOdd[i]->pathMetric + distortion;

								// Update the child of the parent to NULL
								if (activeEven[activeQ[q].second.first]->parent->left == activeEven[activeQ[q].second.first])
									activeEven[activeQ[q].second.first]->parent->left = NULL;
								else
									activeEven[activeQ[q].second.first]->parent->right = NULL;

								// Make the parent of the active node the node in the previous level
								activeEven[activeQ[q].second.first]->parent = activeOdd[i];

								// Update the centroid ID, state ID and quantizer ID
								activeEven[activeQ[q].second.first]->centroidID = quantizer[q].second.second;
								activeEven[activeQ[q].second.first]->stateID = activeQ[q].second.first;
								activeEven[activeQ[q].second.first]->quantizerID = activeQ[q].first;

								// Update the value to be compressed
								activeEven[activeQ[q].second.first]->value2Compr = source[t];
								activeEven[activeQ[q].second.first]->noise = quantizer[q].second.first - source[t]; // New line

								// Add this node as a child to the parent in the previous level
								if (activeOdd[i]->left == NULL)
									activeOdd[i]->left = activeEven[activeQ[q].second.first];
								else
									activeOdd[i]->right = activeEven[activeQ[q].second.first];
							}
						}

					}


				}

			}
			if (t != T - 1)
				for (int i = 0; i < nStates; i++)
					activeOdd[i] = NULL;
		}
		

		quantizer.clear();
		activeQ.clear();
	}
	
	viterbiNode* bestPath = NULL;
	if (T % 2 == 0) {
		// Use the active Even Array
		bestPath = findMin(activeEven);
	}
	else {
		// Use the active Even Array
		bestPath = findMin(activeOdd);
	}

	compressedData = outputBits(bestPath);
	finalCentroids = outputCentroids(bestPath);
	std::cout << bestPath->pathMetric << " State: " << bestPath->stateID << "\n";
	std::cout << "MSE = " << (bestPath->pathMetric/T) << "\n";
	std::cout << "SNR(dB) = " << 10* (log(1/(bestPath->pathMetric / T))/log(10)) << "\n";
	saveNoiseRealizations(bestPath);
	//printTree(root);

	return compressedData;
}


void TCQuantizer::saveNoiseRealizations(viterbiNode* bestPath) {

	std::vector<double> example(T, 0);
	for (int i = 0; i < T; i++)
	{
		example[i] = bestPath->noise;
		bestPath = bestPath->parent;
	} 

	std::ofstream output_file("./noiseRealizations.txt");
	std::ostream_iterator<double> output_iterator(output_file, "\n");
	std::copy(example.begin(), example.end(), output_iterator);
}

std::vector<double> TCQuantizer::decode(std::vector<int> outputBits) {

	std::vector<double> reconstructedValues(T, 0);
	trellisNode * node =  graph->adj[0];
	int usedQ;
	int k = 0;
	int i = 0;
	while(true){
		for (int j = 0; j < outputBits[i]; j++){
			node = node->next;
		}

		usedQ = node->output;
		
		if (nElements > 2) {
			int dec = 0;
			int power2 = 1;
			for (int q = log(nElements) / log(2) - 1; q >= 0; q--) {
				dec += outputBits[i + q + 1] * power2;
				power2 = power2 * 2;
			}
			reconstructedValues[k] = centroids[dec][usedQ];
		}
		else {
			reconstructedValues[k] = centroids[outputBits[i + 1]][usedQ];
		}

		
		node = graph->adj[node->nextNode];
		i = i + 1 + (log(nElements) / log(2));
		k++;
		if (i >= outputBits.size())
			break;
	}

	
	checkCodeQuantizer(reconstructedValues);
	return reconstructedValues;
}

void TCQuantizer::checkCodeQuantizer(std::vector<double> reconstructedValues) {
	for (int i = 0; i < T; i++){
		if (abs(finalCentroids[i] - reconstructedValues[i]) > 0.1) {
			std::cout << "Problem! \n";
			return;
		}
			
	}
}

std::vector<double> TCQuantizer::outputCentroids(viterbiNode* bestPath) {
	std::vector<double> finalCentroids(T, 0);
	int t = T -1;
	while (true){


		finalCentroids[t] = centroids[bestPath->centroidID][bestPath->quantizerID];
		bestPath = bestPath->parent;
		t--;
		if (t < 0)
			break;
	}

	return finalCentroids;
}



std::vector<int> TCQuantizer::outputBits(viterbiNode* bestPath) {
	
	std::vector<int> compressedData(T*(log(nElements)/log(2) + 1), 0);
	int decNum;
	int t = T * ((log(nElements) / log(2)) +1) -1;
	int tTemp = T * ((log(nElements) / log(2)) + 1) - 2 - (log(nElements) / log(2));
	int count =0;
	int bits;
	for (;;) {
		
		bits = 0;

		decNum = bestPath->centroidID;
		
		if (decNum != 0) {
			count  = 0;
			while (decNum > 0) {
			
				compressedData[t] = decNum % 2;
				decNum = decNum / 2;
				count++;
				t--;		
				bits++;
			}
		}
		else {
			for (int p = 0; p < (log(nElements) / log(2)); p++) {
				t--;
				bits++;
			}
			count = (log(nElements) / log(2));
		}

		for (int p = count; p < (log(nElements) / log(2)); p++) {
			t--;
			bits++;
		}		

		
		compressedData[t] = bestPath->input;
		
		bestPath = bestPath->parent;
		t--;
		bits++;
		if (t < 0)
			break;


	}


	return compressedData;
}

std::vector<std::pair<int, std::pair<int, int>>> TCQuantizer::findActiveQuantizers(trellisNode* node) {
	std::vector<std::pair<int, std::pair<int,int>>> activeQ;
	for (int i = 0; i < nBranches; i++) {
		activeQ.push_back(std::make_pair(node->output, std::make_pair(node->nextNode, node->input)));
		node = node->next;
	}

	return activeQ;
}

std::vector<std::pair<int, std::pair<double, int>>> TCQuantizer::createQuantizer(std::vector<std::pair<int, std::pair<int, int>>>  activeQ, double value2Compr) {
	// For all available quantizers
	int quantID, centroid;
	double distortion;
	int flag;
	std::vector<std::pair<int, std::pair<double, int>>> activeValues;
	for (int i = 0; i < activeQ.size(); i++) {
		quantID = activeQ[i].first;
		distortion = 10000;
		// For all elements for each quantizer
		for (int j = 0; j < nElements; j++) {
			// Find the min distortion
			centroid = centroids[j][quantID];

			// Compute the distance
			if (distortion > pow(centroid - value2Compr, 2)) {
				flag = j;
				distortion = pow(centroid - value2Compr, 2);
			}
		}

		activeValues.push_back(std::make_pair(quantID, std::make_pair(centroids[flag][quantID], flag)));
	}

	return activeValues;
}

void TCQuantizer::createPolynomials(int nStates) {

	memory = log(nStates) / log(2);
	int statesId = memory - 2;

	int decNum = h0Values[statesId];
	int i = 0;
	while (decNum > 0) {
		// storing remainder in binary array
		h0[i] = decNum % 2;
		decNum = decNum / 2;
		i++;
	}

	for (int j = i; j < memory + 1; j++)
		h0[j] = 0;


	decNum = h1Values[statesId];
	i = 0;
	while (decNum > 0) {
		// storing remainder in binary array
		h1[i] = decNum % 2;
		decNum = decNum / 2;
		i++;
	}

	for (int j = i; j < memory + 1; j++)
		h1[j] = 0;

}

viterbiNode* TCQuantizer::createNode(int stateID, int quantizerID, double value2Compr, int input) {

	viterbiNode* newNode = new viterbiNode();
	newNode->stateID = stateID;
	newNode->quantizerID = quantizerID;
	newNode->value2Compr = value2Compr;
	newNode->noise;
	newNode->centroidID;
	newNode->input = input;
	newNode->pathMetric = 0;
	newNode->parent = NULL;
	newNode->left = NULL;
	newNode->right = NULL;
	return newNode;
}

void TCQuantizer::printTree(viterbiNode * root) {
	
	if (root == NULL) {
		std::cout << "G" << " \n";
		return;
	}
		
	std::cout << root->pathMetric << " State: " << root->stateID<< "\n";
	printTree(root->left);
	printTree(root->right);




}

viterbiNode* TCQuantizer::findMin(viterbiNode * leafs[]) {
	double min = leafs[0]->pathMetric;
	int flag = 0;
	for (int i = 1; i < nStates; i++) {
		if (leafs[i]->pathMetric < min) {
			min = leafs[i]->pathMetric;
			flag = i;
		}
	}

	return leafs[flag];
}
