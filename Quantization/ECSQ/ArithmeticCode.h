#pragma once
#include<iostream>
#include<vector>
//const int No_of_chars = 16;
//const int No_of_symbols = (No_of_chars + 1);  /* Total number of symbols      */
//using namespace std;
class ArithmeticCode
{	// NEW
	int dataSize;
	int No_of_chars;
	int No_of_symbols;


public:

	// NEW
	//Output indices
	std::vector<int> indicesHat;


	ArithmeticCode(int nSymbols, int dataLen);



	// === Parameters
	// File names
	FILE* fin = NULL;
	FILE* fout = NULL;
	char outFile[30] = "gaussian_compress_bin";
	char recFile[18] = "gaussian_hat";


	// Arithmetic coding parameters
	int EOF_symbol;    /* Index of EOF symbol          */
	/* SIZE OF ARITHMETIC CODE VALUE */
	int Code_value_bits = 16;           /* Number of bits in a code value   */
	typedef long code_value;            /* Type of an arithmetic code value */
	int Top_value = (((long)1 << Code_value_bits) - 1);  /* Largest code value */


	/* HALF AND QUARTER POINTS IN THE CODE VALUE RANGE */

	int First_qtr = (Top_value / 4 + 1);   /* Point after first quarter        */
	int Half = (2 * First_qtr);    /* Point after first half           */
	int Third_qtr = (3 * First_qtr);     /* Point after third quarter        */


	// Model Parameters
	/* TRANSLATION TABLES BETWEEN CHARACTERS AND SYMBOL INDICES. */

	int* char_to_index;        /* To index from character      */
	int* index_to_char; //[No_of_symbols + 1]; /* To char from index    */

	/* CUMULATIVE FREQUENCY TABLE. */

	int Max_frequency = 16383;         /* maximum allowed frequency    */
										   /* count 2^14 - 1               */

	int* cum_freq;// [No_of_symbols + 1] ;          /* Cumulative symbol frequencies*/


	int* freq;// [No_of_symbols + 1] ;

	int buffer, bits_to_go, garbage_bits;

	long low, high, value;
	long bits_to_follow;


	// ====== For encoder

	int lenBits = 0;

	void start_encoding(); //
	void start_outputing_bits();//
	void encode(int data[]);
	void encode_symbol(int symbol);//
	void bit_plus_follow(int bit);//
	void done_outputing_bits();//
	void output_bit(int bit);//
	void done_encoding();//


	// ============================================ For Decoding ======================================== //
	std::vector<int> decoder();
	void start_inputing_bits();
	int input_bit();
	void start_decoding();
	int decode_symbol();




	// ============================================ For the model ======================================== //
	void start_model();//
	void update_model(int symbol);//
};
