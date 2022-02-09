#pragma once
#include<iostream>
const int No_of_chars = 2;
const int No_of_symbols = (No_of_chars + 1);  /* Total number of symbols      */
const int  memory = 4;
//using namespace std;
class ArithmeticCode
{


	public:
		ArithmeticCode();
		
		

		// === Parameters
		// File names
		FILE* fin = NULL;
		FILE* fout = NULL;
		char inFile[9] = "bin_data";
		char outFile[30] = "bin_data_compress_bin";
		char recFile[18] = "bin_data_hat";


		// Arithmetic coding parameters
		int EOF_symbol = (No_of_chars + 1);    /* Index of EOF symbol          */
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

		int char_to_index[No_of_chars];        /* To index from character      */
		unsigned char index_to_char[No_of_symbols + 1]; /* To char from index    */
		int previous[memory];

		/* CUMULATIVE FREQUENCY TABLE. */

		int Max_frequency = 16383;         /* maximum allowed frequency    */
											   /* count 2^14 - 1               */

		int cum_freq[(1 << memory)][No_of_symbols + 1];          /* Cumulative symbol frequencies*/
		
		
		int freq[(1 << memory)][No_of_symbols + 1];

		int buffer, bits_to_go, garbage_bits;

		long low, high, value;
		long bits_to_follow;


		// ====== For encoder

		int lenBits = 0;

		void start_encoding(); //
		void start_outputing_bits();//
		void encode();
		void encode_symbol(int symbol);//
		void bit_plus_follow(int bit);//
		void done_outputing_bits();//
		void output_bit(int bit);//
		void done_encoding();//
	

		// ============================================ For Decoding ======================================== //
		void decoder();
		void start_inputing_bits();
		int input_bit();
		void start_decoding();
		int decode_symbol();




		// ============================================ For the model ======================================== //
		void start_model();//
		void update_model(int symbol);//
		void shiftMemory(int symbol);
		int bin2dec();
};

