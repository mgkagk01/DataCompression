#include<vector>
#include<math.h>
#include<iostream>
#include "utility.h"
#include <fstream>
#include <bitset>
#pragma once

class GolmobCode{
public:
	GolmobCode(int const n, float p, int k);
	
	// Attributes
	int lenSource;
	bool compressedData[8];
	int m;
	int lenXr;
	void encoder(int source[]);
	void decoder(bool compressed[], int size);
};

