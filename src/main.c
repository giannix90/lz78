
/*
* Basic implementation of LZ78 compression algorithm 
*
* Copyright (C) 2016 giannix90 <gianni.pollina1@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <sys/types.h>

#include "lz78.h"


/*
	command to compress ./lz78 -o file.lz78 -c file.txt
*/

void print_bytes(const unsigned char* buf, int len){
	int i;
	for(i=0;i<len-1;i++)
		printf("%02X",buf[i]);
	printf("%02X",buf[len-1]);
	
}

int main(int argc,char* argv[])
{
	struct bitio* file;
	struct bitio* file_to_read;
	int option;
	char * arguments;
	lz78_compressor* compressor;
	lz78_decompressor* decompressor;


	compressor=malloc(sizeof(lz78_compressor));
	decompressor=malloc(sizeof(lz78_decompressor));
	
	while ((option = getopt(argc, argv,"c:d:l:i:o:h")) != -1) {
	/* int getopt(int argc, char * const argv[],const char *optstring);
	
       	optstring is a string containing the legitimate option characters.
       	If such a character is followed by a colon, the option requires an
       	argument

	optarg is a string setted after the call to getopt, and contain the pointer to passed argument

	optopt is a char that contain the value of the passed option
	*/	       
 
		switch (option) {

		case 'c':
			arguments=optarg;
			//printf("File da comprimere : %s \n",arguments);
			//init_struct=malloc(sizeof(struct lz78_compressor));
			init_compressor(arguments,compressor);
			printf("Spazio:::::: %d\n",' ');
			printf("File da comprimere2 : %s \n",compressor->file_to_compress);
			//huge prime number=101
			if(hash_init(6700417,compressor)){ //82MB used //7,5 Mb of Hash Table in main memory with prime=104729
				printf("Hash initialization error\n");
				_exit(-1);			
			}
			file=bit_open(compressor->file_to_compress,0);
			if(file==NULL){
				printf("Error open file\n");
				_exit(-1);
			}
			/*----------*/
			//init_struct->output_file="output.lz78";
			

			compress(arguments,compressor,file);

			
			//print_hash_table();			

			break;
		case 'd':
			arguments=optarg;
			init_decompressor(arguments,decompressor);
			//file_to_read=bit_open(arguments,0);
			file=bit_open(decompressor->output_file,1);
			if(file==NULL){
				file=bit_open("decompressed.txt",1);
			}
			
			array_init(6700417,decompressor);//4793//10007
			
			
			decompress(file_to_read,file,decompressor);
			break;
		
		case 'l':
			arguments=optarg;
			struct bitio*file=bit_open(arguments,0);
			uint64_t in;
			char a=0;
			for(int i=0;i<=50;i++){
				bit_read(file,(u_int)8,&in);
				a=(char)(in&((1UL<<8)-1));
			printf(" %c-%d ",a,(int) a);			
			}
			break;

		case 'i':
			
			break;

		case 'h':
			printf("lz78 -o [OUTPUT_FILE] -c [FILE_TO_COMPRESS] \n");
			printf("lz78 -o [OUTPUT_FILE] -d [FILE_TO_DECOMPRESS] \n");
			printf("lz78 -l [COMPRESSED_FILE] \n");
			printf("lz78 -h help \n");
			
			break;
		
		case 'o':
			arguments=optarg;
			decompressor->output_file=compressor->output_file=arguments;
			
			
			break;
		
		case '?':
        		if (optopt == 'c')
          			fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        		else if (isprint (optopt))
          			fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        		else
          			fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
       			return 1;
      	default:
        		abort ();
		}
	}

return 0;
	
}
