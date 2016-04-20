//Tyler Fitzgerald - huffman Encoding and Decoding

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
// string.h only used to deal with command line input and output
#include <string.h>

#define byte 256

typedef struct node{
	int freq;
	struct node *left;
	struct node *right;
	struct node *back;
} node;

typedef struct code{
	int length;
	unsigned char *encoding;
} code;




int main(int argc, char *argv[]){
	if(argc < 3 || argc > 4){ 	//3 or 4 arguments as required to compress or uncompress
		printf("Usage:\n\nCompress:\n./huff [-c] [saved filename]\n     \nor\n\nDecompress:\n./huff [-u] [compressed filename] [desired uncompressed filename]\n");
		exit(0);
	}
	if( strcmp(argv[1],"-c") == 0){ //compressing
		if(argc != 3){
			printf("Usage: ./huff [-c] [saved filename]\n");
			exit(0);	
		}
		//create array to hold frequency of bytes 0-255. Initialize all counts to 0 (using calloc)
		int *freq = calloc(byte,sizeof(int));
		FILE *fpt = fopen(argv[2],"rb");
		if (fpt == NULL){ //error. shouldn;t occur
			printf("File read from returned NULL");
			exit(0);
		}
		int c;
		//loop through file byte by byte and increment byte frequenciy count accordingly
		while( (c = fgetc(fpt)) != EOF){
			freq[ c ]++;
		}
		//initialize nodeArray with frequency values and null left, right, and back pointers
		int x;
		node *nodeArray = calloc(byte,sizeof(node));
		for( x=0; x < byte; x++){
			nodeArray[x].freq = freq[x];
			nodeArray[x].left = NULL;
			nodeArray[x].right = NULL;
			nodeArray[x].back = NULL;
		} 
		int nodeCount = byte; //initially one node for each char value 0-255. Even if not all are used

		node *small1,*small2;
		int small1Val, small2Val;
		int nullBackCount;
		//creates the binary tree via huffman encoding compression algorithm
		while(1){
			small1 = NULL;
			small2 = NULL;
			small1Val = INT_MAX;
			small2Val = INT_MAX;
			nullBackCount = 0;
			for( x=0; x < nodeCount; x++){ //cycle through all nodes
				if( nodeArray[x].back == NULL && nodeArray[x].freq != 0){ //examine nodes whose corresponding frequncy indicates they're in txt file and the node must not have a back pointer
					nullBackCount++;
					if( nodeArray[x].freq <= small1Val ){ //found new possible of the smallest 2 nodes 
						small2 = small1;
						if(small2 != NULL) small2Val = small2->freq;					
						small1 = &nodeArray[x];
						small1Val = small1->freq;
					}
					else if( nodeArray[x].freq <= small2Val){ //found new possible of the smallest 2 nodes 
						small2 = &nodeArray[x];
						small2Val = small2->freq;
					}
				}
			}
			if(nullBackCount == 1) break; //only one node that does not have a back pointer. tree complete
			nodeArray = realloc(nodeArray,(nodeCount+1) * sizeof(node) ); //create new node
			nodeArray[nodeCount].freq = small1Val +small2Val; //new node freq = freq of two smallest free nodes found
			//set new node to point to smallest two available nodes found (l&R) and it's back pointer to null
			nodeArray[nodeCount].left = small1; 
			nodeArray[nodeCount].right = small2;
			nodeArray[nodeCount].back = NULL;
			//set the smallest two nodes back pointers to the newly created node
			small1->back = &nodeArray[nodeCount];
			small2->back = &nodeArray[nodeCount];
			nodeCount++;
		}	
		//binary tree completed. Stored and interconnected in nodeArray

		//Initialize codeArray for each node to null
		code codeArray[byte];
		for(x=0; x < byte; x++){
			codeArray[x].encoding = NULL;
		}

		//loop through nodeArray[0-255], traversing with the back pointers. store encoding in codeArray[0-255] (traversing from the leafs to root leads to backwards code, it is reversed)
		int charCount,y,y2;
		node *curr,*prev;	
		for(x=0; x < byte; x++){ //loop through each char (0-255) to store its code
			if(nodeArray[x].freq == 0) continue; //this char not in file to encode
			curr = &nodeArray[x];
			charCount = 0;
			while(curr->back != NULL){
				//traverse bacwards 
				charCount++;
				prev = curr;
				curr = prev->back;
				codeArray[x].encoding = realloc(codeArray[x].encoding, charCount);
				if(prev == curr->left ) codeArray[x].encoding[charCount - 1] = 0; // 0 = used left pointer to trverse
				else codeArray[x].encoding[charCount - 1] = 1; // 1 = used right pointer to trverse
				codeArray[x].length = charCount; //store one digit (0 or 1) for code corresponding to current char
			}
			//back == null. full code stored in reverse

			//reverse the codes
			unsigned char codeHold[codeArray[x].length];
			for(y=0; y < codeArray[x].length; y++){
				codeHold[y] = codeArray[x].encoding[y];
			}
			y2 = 0;
			for(y=codeArray[x].length - 1; y >=0; y--){
				codeArray[x].encoding[y2] = codeHold[y];
				y2++;
			}
		}
		//encodings and their respective lengths are stored in codeArray[0-255]	
		
		//begin output to file
		FILE *fpt2 = fopen(strcat(argv[2],".huff"),"w+");
		if (fpt2 == NULL){
			printf("File pointer returned NULL");
			exit(0);
		}
		//first the 256 int array holding the char-frequencies is written to file
		//can use the same exact methods as above in decompression to create the code arrays
		fwrite(freq,sizeof(int),byte,fpt2);
	
		//now rewind the read from file to read through byte by byte again. Write the proper encoding of each byte 
		rewind(fpt);
		int breakOut = 0;
		int count = 0;
		unsigned char byteOut = 0x00;
		unsigned char *compressed;
		int totalBytes = 0;
		//extra bits is needed to indicate the amount of padding with 0's on the very last byte
		short extraBits = 0;
		c = fgetc(fpt);
		while( 1 ){
				for(x =0; x < 8; x++){
					if( count >= codeArray[c].length){
						c = fgetc(fpt);
						if( c == EOF){ //end of file reached. determine padded 0's to write
							breakOut = 1;
							extraBits = 8 - (short)x;
							if(extraBits == 8) extraBits = 0;
							byteOut = byteOut << (extraBits - 1);
							break;
						}
						count = 0;
					}
					byteOut = byteOut | codeArray[c].encoding[count];
					if(x != 7) byteOut = byteOut << 1;
					count++;
				}
				totalBytes++;
				//add byte to compressed file output
				compressed = realloc(compressed,totalBytes);
				compressed[totalBytes-1] = byteOut;
				if(breakOut == 1) break;
				byteOut = 0x00; //set back to 0
		}
		//write one short after the freq array to the file
		// this short indicates the amount of padded 0's on the last byte. Range (0-7)
		fwrite(&extraBits,sizeof(unsigned short),1,fpt2);
		fwrite(compressed,sizeof(unsigned char),totalBytes,fpt2);
		//close files and return
		fclose(fpt2);
		fclose(fpt);
	}
	else if( strcmp(argv[1],"-u") == 0){ //decompressing
		if(argc != 4){
			printf("Usage: ./huff [-u] [compressed filename] [desired uncompressed filename]\n");
			exit(0);	
		}
		FILE *fpt = fopen(argv[2],"rb");
		if (fpt == NULL){
			printf("File read from returned NULL");
			exit(0);
		}
		int *freq = calloc(byte,sizeof(int));
		fread(freq, byte, sizeof(int), fpt); //read in first 256 bytes of encoded file which stores the frequency array. This frequency array holds the frequency of ech car (0-255) in the un-encoded file. This is used to create the huffman binary tree
		short *padding = (short *) calloc(1,sizeof(short));
		fread(padding, 1, sizeof(short), fpt); //read the next short of the file which stores the value of number of padded 0 bits at end of the file

		int c;
		unsigned char *encoded;
		int count = 0;		
		while( (c = fgetc(fpt)) != EOF){
			count++;
			encoded = realloc(encoded,count);
			encoded[count-1] = (unsigned char) c;
		}

		//compress
		//initialize nodeArray
		int x;
		node *nodeArray = calloc(byte,sizeof(node));
		for( x=0; x < byte; x++){
			nodeArray[x].freq = freq[x];
			nodeArray[x].left = NULL;
			nodeArray[x].right = NULL;
			nodeArray[x].back = NULL;
		} 
		int nodeCount = byte; //initially one node for each char value 0-255. Even if not all are used

		node *small1,*small2;
		int small1Val, small2Val;
		int nullBackCount;
		//creates the binary tree via huffman encoding compression algorithm
		while(1){
			small1 = NULL;
			small2 = NULL;
			small1Val = INT_MAX;
			small2Val = INT_MAX;
			nullBackCount = 0;
			for( x=0; x < nodeCount; x++){ //cycle through all nodes
				if( nodeArray[x].back == NULL && nodeArray[x].freq != 0){ //examine nodes whose corresponding frequncy indicates they're in txt file and the node must not have a back pointer
					nullBackCount++;
					if( nodeArray[x].freq <= small1Val ){ //found new possible of the smallest 2 nodes 
						small2 = small1;
						if(small2 != NULL) small2Val = small2->freq;					
						small1 = &nodeArray[x];
						small1Val = small1->freq;
					}
					else if( nodeArray[x].freq <= small2Val){ //found new possible of the smallest 2 nodes 
						small2 = &nodeArray[x];
						small2Val = small2->freq;
					}
				}
			}
			if(nullBackCount == 1) break; //only one node that does not have a back pointer. tree complete
			nodeArray = realloc(nodeArray,(nodeCount+1) * sizeof(node) ); //create new node
			nodeArray[nodeCount].freq = small1Val +small2Val; //new node freq = freq of two smallest free nodes found
			//set new node to point to smallest two available nodes found (l&R) and it's back pointer to null
			nodeArray[nodeCount].left = small1; 
			nodeArray[nodeCount].right = small2;
			nodeArray[nodeCount].back = NULL;
			//set the smallest two nodes back pointers to the newly created node
			small1->back = &nodeArray[nodeCount];
			small2->back = &nodeArray[nodeCount];
			nodeCount++;
		}	
		//binary tree completed. Stored and interconnected in nodeArray

		//Initialize codeArray
		code codeArray[byte];
		for(x=0; x < byte; x++){
			codeArray[x].length = 0;
			codeArray[x].encoding = NULL;
		}

		//loop through nodeArray[0-255], traversing with the back pointers. store encoding in codeArray[0-255] (traversing from the leafs to root leads to backwards code, it is reversed)
		int charCount,y,y2;
		node *curr,*prev;	
		for(x=0; x < byte; x++){ //loop through each char (0-255) to store its code
			if(nodeArray[x].freq == 0) continue; //this char not in file to encode
			curr = &nodeArray[x];
			charCount = 0;
			while(curr->back != NULL){
				//traverse bacwards 
				charCount++;
				prev = curr;
				curr = prev->back;
				codeArray[x].encoding = realloc(codeArray[x].encoding, charCount);
				if(prev == curr->left ) codeArray[x].encoding[charCount - 1] = 0; // 0 = used left pointer to trverse
				else codeArray[x].encoding[charCount - 1] = 1; // 1 = used right pointer to trverse
				codeArray[x].length = charCount; //store one digit (0 or 1) for code corresponding to current char
			}
			//back == null. full code stored in reverse


			//reverse the codes
			unsigned char codeHold[codeArray[x].length];
			for(y=0; y < codeArray[x].length; y++){
				codeHold[y] = codeArray[x].encoding[y];
			}
			y2 = 0;
			for(y=codeArray[x].length - 1; y >=0; y--){
				codeArray[x].encoding[y2] = codeHold[y];
				y2++;
			}
		}

		//encodings and their respective lengths are stored in codeArray[0-255]
		//end of compression

		//calculate the total freq of all chars ( = to # bytes of unencoded file to be created)
		int totalFreq = 0;		
		for(x=0; x<byte; x++){
			totalFreq += freq[x];
		}

		x = 4;
		// begin to scan through encoded bit output the corresponding bytes to output array
		unsigned char *output;
		int outputCount = 0;
		unsigned char *bitArray;
		unsigned char currByte;
		int byteCount = 0;
		int bitCount = 0;
		unsigned char bit;
		int z;
		int matches = 0;
		while(1){
			if(byteCount > count) break; //all encoded bytes decoded. exit while and write file
			currByte = encoded[byteCount];
			for(x=0;x < 8; x++){ //bit loop for each byte
				bitCount++;
				bitArray = realloc(bitArray,bitCount);
				//store next bit value in bit (unsigned char)
				switch(x){ //storing 1 bit of current encdoded byte in bit (bit is an unsgned char)
					case 0:
						bit = currByte &  0x80;
						bit = bit >> 7;
						break;
					case 1:
						bit = currByte &  0x40;
						bit = bit >> 6;
						break;
					case 2:
						bit = currByte &  0x20;
						bit = bit >> 5;
						break;
					case 3:
						bit = currByte &  0x10;
						bit = bit >> 4;
						break;
					case 4:
						bit = currByte &  0x8;
						bit = bit >> 3;
						break;
					case 5:
						bit = currByte &  0x4;
						bit = bit >> 2;
						break;
					case 6: 						
						bit = currByte &  0x2;
						bit = bit >> 1;
						break;
					case 7:
						bit = currByte &  0x1;
						byteCount++;
						break;
				}
				bitArray[bitCount - 1] = bit; // add bit to 
				//loop through each code entry and see if it's length matches the current code length				
				for(y=0; y<byte; y++){ // y = current code entry
					if(bitCount == codeArray[y].length ){ //matches in length
						//loop through the length and see if each bit is a match
						for(z=0; z < bitCount; z++){
							if(bitArray[z] != codeArray[y].encoding[z]){ //does not match
								matches = 0;	
								break;
							}
							matches = 1;
						}
						if(matches == 1){ //code matches output the byte corresping to the encoding
							bitCount = 0;
							matches = 0;
							outputCount++;
							output = realloc(output, outputCount);
							output[outputCount - 1] = y;
							break;
						}
						else{
							continue;
						}
					}			
				}
				if(outputCount == totalFreq){ //if total size of output is eual to the known size of un-encoded file. Output the decoded file stored in output array
					FILE *fpt2 = fopen(argv[3],"w+");
					if (fpt2 == NULL){ //error. shouldn't occur
						printf("File pointer returned NULL");
						exit(0);
					}
					//write byteOut uncompressed file to desired uncompressed filename
					fwrite(output,1,outputCount,fpt2);
					fclose(fpt2);
				}
			}		
		}
	}
	else{
		printf("Usage:\n\nCompress:\n./huff [-c] [saved filename]\n     \nor\n\nDecompress:\n./huff [-u] [compressed filename] [desired uncompressed filename]\n");
		exit(0);
	}
	return 1;
}

