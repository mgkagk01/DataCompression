#include "ArithmeticCode.h"

ArithmeticCode::ArithmeticCode() {

}

void ArithmeticCode::encode(){

    int ch; int symbol;

    // Open files to read and store data
    fopen_s(&fin, inFile, "rb");
    fopen_s(&fout, outFile, "wb");
   

    // Start the model
    start_model();

    // Start outputing bits
    start_outputing_bits();

    // Start encoding
    start_encoding();

  
    for (;;) {
        ch = getc(fin);
        if (ch == EOF)
            break;
              
   
        
        symbol = char_to_index[ch];
        encode_symbol(symbol);
        update_model(symbol);
        shiftMemory(symbol);

    }



    encode_symbol(EOF_symbol);
    done_encoding();            /* Send the last few bits.  */
    done_outputing_bits();


    fclose(fin);
    fclose(fout);


}



/* INITIALIZE FOR BIT OUTPUT. */

void ArithmeticCode::start_outputing_bits(){

    buffer = 0;                           /* Buffer is empty to start   */
    bits_to_go = 8;                       /* with.                      */
}


/*  OUTPUT A BIT */

void ArithmeticCode::output_bit(int bit){
    lenBits++;
    buffer >>= 1;                         /* Put bit in top of buffer   */
    if (bit) 
        buffer |= 0x80;

    bits_to_go -= 1;
    if (bits_to_go == 0) {                /* Output buffer if it is     */
        putc(buffer, fout);              /* now full.                  */
        bits_to_go = 8;
        
    }
}


/* FLUSH OUT THE LAST BITS. */

void ArithmeticCode::done_outputing_bits(){
    putc(buffer >> bits_to_go, fout);
}





/* START ENCODING A STREAM OF SYMBOLS. */

void ArithmeticCode::start_encoding(){
    low = 0;                     /* Full code range.                 */
    high = Top_value;
    bits_to_follow = 0;              /* No bits to follow next.          */
}


/* ENCODE A SYMBOL. */

/* Symbol to encode                 */ /* Cumulative symbol frequencies    */
void ArithmeticCode::encode_symbol(int symbol){



    int dec = bin2dec();

    long range;                     /* Size of the current code region  */
    range = (long)(high - low) + 1;

    high = low + (range * cum_freq[dec][symbol - 1]) / cum_freq[dec][0] - 1;/* to that allocted to  */

    low = low + (range * cum_freq[dec][symbol]) / cum_freq[dec][0];
   

    

    for (; ;) {                     /* Loop to output bits.             */
        if (high < Half) {
            bit_plus_follow(0);      /* Output 0 if in low half.         */

        }
        else if (low >= Half) {     /* Output 1 if in high half.        */
            bit_plus_follow(1);
            low -= Half;
            high -= Half;            /* Subtract offset to top.          */
        }
        else if (low >= First_qtr   /* Output an opposite bit           */
            && high < Third_qtr) {  /* later if in middle half.         */
            bits_to_follow += 1;
            low -= First_qtr;        /* Subtract offset to middle.       */
            high -= First_qtr;
        }
        else break;                 /* Otherwise exit loop.             */
        low = 2 * low;
        high = 2 * high + 1;            /* Scale up code range.             */
    }
}


/* FINISH ENCODING THE STREAM */

void ArithmeticCode::done_encoding(){
    bits_to_follow += 1;                      /* Out two bits that       */
    if (low < First_qtr) bit_plus_follow(0);
    else bit_plus_follow(1);                 /* the current code range  */
}                                            /* contains.               */


/* OUTPUT BITS PLUS FOLLOWING OPPOSITE BITS */

void ArithmeticCode::bit_plus_follow(int bit){

    output_bit(bit);                /* Output the bit.                  */
    while (bits_to_follow > 0) {
        output_bit(!bit);               /* Output bits_to_follow            */
        bits_to_follow -= 1;        /* opposite bits. Set               */
    }                               /* bits_to_follow to zero.          */
}




















 // === For decoding
// DECODER
void ArithmeticCode::decoder() {
    //int i, j;
    int ch; int symbol;


    // --- Open files to read and store data
    fopen_s(&fin, outFile, "rb");
    fopen_s(&fout, recFile, "wb");

    // --- Set up other modules    
    start_model();           
    start_inputing_bits();
    start_decoding();

    for (;;) {
        symbol = decode_symbol();
        if (symbol == EOF_symbol)
            break;      /* Exit loop if EOF symbol  */
        ch = index_to_char[symbol];  /* Translate to a char.     */
        putc(ch, fout);                    /* White that character.    */
        update_model(symbol);
        shiftMemory(symbol);
    }



    fclose(fin);
    fclose(fout);
}


void ArithmeticCode::start_inputing_bits(){
    bits_to_go = 0;                       /* Buffer starts out with     */
    garbage_bits = 0;                     /* no bits in it.             */
}


/*  INPUT A BIT */
int ArithmeticCode::input_bit(){
    int t;
    if (bits_to_go == 0) {                /* Read the next byte if no  */
        buffer = getc(fin);             /* bits are left in buffer.  */
        if (buffer == EOF) {
            garbage_bits += 1;
            if (garbage_bits > Code_value_bits - 2) {
                printf("Bad input file\n");
                exit(-1);
            }
        }
        bits_to_go = 8;
    }
    t = buffer & 1;                        /* Return the next bit from  */
    buffer >>= 1;                        /* the bottom of the byte.   */
    bits_to_go -= 1;
    return t;
}

void ArithmeticCode::start_decoding(){
    int i;
    value = 0;                            /* Input bits to fill the     */
    for (i = 1; i <= Code_value_bits; i++) {/* code value.                */
        value = 2 * value + input_bit();
    }
    low = 0;                     /* Full code range.                 */
    high = Top_value;
}


/* DECODE THE NEXT SYMBOL. */

int ArithmeticCode::decode_symbol(){

    int dec = bin2dec();

    long range;                   /* Size of current code region       */
    int cum;                      /* Cumulative frequency calculated   */
    int symbol;                   /* Symbol decoded                    */
    range = (long)(high - low) + 1;
    cum = (((long)(value - low) + 1) * cum_freq[dec][0] - 1) / range;

    for (symbol = 1; cum_freq[dec][symbol] > cum; symbol++);/* Then find symbol. */

    high = low + (range * cum_freq[dec][symbol - 1]) / cum_freq[dec][0] - 1;  /* to that allocated  */
    low = low +  (range * cum_freq[dec][symbol]) / cum_freq[dec][0];

    for (; ;) {                   /* Loop to get rid of bits.          */
        if (high < Half) {
            /* nothing */         /* Expand low half                   */
        }
        else if (low >= Half) {     /* Expand high half                  */
            value -= Half;
            low -= Half;           /* Subtract offset to top.           */
            high -= Half;
        }
        else if (low >= First_qtr   /* Expand middle half                */
            && high < Third_qtr) {
            value -= First_qtr;
            low -= First_qtr;      /* Subtract offset to middle         */
            high -= First_qtr;
        }
        else break;               /* Otherwise exit loop.              */
        low = 2 * low;
        high = 2 * high + 1;          /* Scale up code range.              */
        value = 2 * value + input_bit();   /* Move in next input bit.      */
    }
    return symbol;
}


int ArithmeticCode::bin2dec(){
    int dec = 0;
    for (int i = 0; i < memory; i++){
        dec += previous[i] * (1 << i);
    }
    return dec;
}



// for model
void ArithmeticCode::start_model() {



    for (int i = 0; i < No_of_chars; i++) {   /* Set up tables that         */
        char_to_index[i] = i + 1;     // int      /* translate between symbol   */
        index_to_char[i + 1] = i;  //unsigned char  /* indices and characters.    */
    }
  

    for (int i = 0; i < (1 << memory); i++) {
        for (int j = 0; j <= No_of_symbols; j++){
            cum_freq[i][j] = No_of_symbols - j;
        }
    }   



    for (int i = 0; i < (1 << memory); i++) {
        for (int j = 0; j <= No_of_symbols; j++) {
            freq[i][j] = 1;
        }
        freq[i][0] = 0;
    }





    for (int i = 0; i < memory; i++){
        previous[i] = 0;
    }

   
}

void  ArithmeticCode::update_model(int symbol){

    int dec = bin2dec();
    /*if (dec > 0)
        std::cout << "hello";*/
    int i, cum, ch_i, ch_symbol;
    if (cum_freq[dec][0] > Max_frequency)
    {
        cum = 0;

        for (i = No_of_symbols; i >= 0; i--){

            freq[dec][i] = (freq[dec][i] + 1) / 2;
            cum_freq[dec][i] = cum;
            cum += freq[dec][i];
        }
    }

    for (i = symbol; freq[dec][i] == freq[dec][i - 1]; i--);
    if (i < symbol)
    {
        ch_i = index_to_char[i];
        ch_symbol = index_to_char[symbol];
        index_to_char[i] = ch_symbol;
        index_to_char[symbol] = ch_i;
        char_to_index[ch_i] = symbol;
        char_to_index[ch_symbol] = i;
    }

    freq[dec][i]++;
    while (i > 0)
    {
        i--;
        cum_freq[dec][i]++;
    }

  

}


void ArithmeticCode::shiftMemory(int symbol) {

    for (int i = 0; i < memory - 1; i++){
        previous[i] = previous[i + 1];
    }
    previous[memory - 1] = symbol - 1;


}