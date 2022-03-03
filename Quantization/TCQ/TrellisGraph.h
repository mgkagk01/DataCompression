#pragma once
#include<vector>
#include <iostream>
const int nStates = 4;
// Create the Finite State Machine

typedef struct node {
	int input;
	int nextNode;
	int output;
	node* next;

}trellisNode;

class TrellisGraph
{
	
public:
	//int nStates;
	// Constructor
	TrellisGraph(int h0[], int h1[], int memory);
	
	// Adjecency List
	trellisNode* adj[nStates];
	//trellisNode* adj;

	// Create a node
	trellisNode* createNode(int nextNode, int input, int output);

	int bin2dec(std::vector<int> vector);
	

};

