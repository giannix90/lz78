
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

void 
print_info(info* in_info,struct bitio * file,int show, int * integrity)
{
		/*get algorithm type*/
	if(show){
		printf("%s", "	-Algorithm: ");
		for(int i=0;i<4;i++){
		
			printf("%c", in_info->alg_type[i]);
			
		}
	printf("\n");

	printf("%s: %d\n","	-Size of dictionary",in_info->dictionary_size);

	printf("%s: %d\n","	-Size of each symbol: ",in_info->symbol_size);

	printf("%s : %s\n","	-File name",in_info->file_name);

	printf("%s :%ld bytes\n","	-Size of the original file",in_info->original_size);

	int fd = fileno(file->f); // get fd from FILE * pointer
	struct stat info_on_file;
	if(fstat(fd,&info_on_file))
			printf("Error in readinf file stat\n");

	printf("%s %ld byte\n","	-Compressed size: ",info_on_file.st_size); 

	printf("%s %f %c \n","	-Compression ratio: ", ((float)(1-((float)info_on_file.st_size/(float)in_info->original_size))*100)>=0?((float)(1-((float)info_on_file.st_size/(float)in_info->original_size))*100):0,0x25);

	char text[100];
	struct tm *t = localtime(&in_info->time);
	strftime(text, sizeof(text)-1, "%d %m %Y %H:%M", t);
	printf("	-Time of last modified: %s \n", text);

	//get the checksum
	printf("%s","	-SHA-1 checksum:" );
	for(int i=0;i<20;i++){

		printf(" %2x", (unsigned char)in_info->sha1[i]);
				
	}
	printf("\n");

	printf("%s %d\n","	-Number of symbol into the file: ",in_info->number_of_symbols);

	}

	
	//Calculate the new SHA-1 in order to have integrity
	
	long old_index=ftell(file->f);//save the curren pointer to position of the file
	info * tmp_i=malloc(sizeof(info));
	getSHA1(88,in_info->number_of_symbols,file,tmp_i); //I do the SHA-1 over the whole file


	if(integrity)
		*integrity=strncmp((const char*)tmp_i->sha1,(const char*)in_info->sha1,20);


	if(show)
		printf("%s %s\n\n","	-Status (Integrity): ",strncmp((const char*)tmp_i->sha1,(const char*)in_info->sha1,20)?"The file is corrupted":"OK");


	fseek(file->f,old_index,SEEK_SET); // restore the pointer of the file

}



void 
print_bytes(const unsigned char* buf, int len)
{
	int i;
	for(i=0;i<len-1;i++)
		printf("%02X",buf[i]);
	printf("%02X",buf[len-1]);
	
}



void
init_default(lz78_compressor * c,lz78_decompressor * d)
{
	c->d_max=65535;
	c->output_file="compressed.txt";
	c->counter_child_tree=0; 
	c->number_of_code=0;

	d->d_max=65535;
	d->counter_child_tree=0; 
	d->number_of_code=0;
}


int main(int argc,char* argv[])
{
	struct bitio* file;
	struct bitio* file_to_read;
	int option;
	char * arguments;
	lz78_compressor* compressor;
	lz78_decompressor* decompressor;
	int o=0;


	compressor=malloc(sizeof(lz78_compressor));
	decompressor=malloc(sizeof(lz78_decompressor));

	init_default(compressor,decompressor);

	printf("\n");
	
	while ((option = getopt(argc, argv,"c:d:l:s:o:h")) != -1) {
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
			char * file_name=malloc(strlen(arguments));
			
			init_compressor(strcpy(file_name,arguments),compressor);
			printf("\n");
			printf("File to compress : %s \n",compressor->file_to_compress);

			

			if(hash_init(SIZE_OF_HASH_TABLE,compressor)){ //82MB used //7,5 Mb of Hash Table in main memory with prime=104729
				printf("Hash initialization error\n");
				_exit(-1);			
			}
			file=bit_open(compressor->file_to_compress,0);
			if(file==NULL){
				printf("Error open file\n");
				_exit(-1);
			}
			/*----------*/

			compress(arguments,compressor,file);
			printf("%s %s %s\n",compressor->file_to_compress," compressed into :",compressor->output_file );
					

			break;
		case 'd':
			arguments=optarg;
			char tmp_str[50];
			init_decompressor(arguments,decompressor);	

			file_to_read=bit_open(arguments,0);
			info * j=getinfo(file_to_read);
			decompressor->d_max=j->dictionary_size;
			//create the output file name
			if(o){
				
				strcpy(tmp_str,decompressor->output_file);
				strcat(tmp_str,"/");
				strcat(tmp_str,j->file_name);
			}

			file=bit_open(o?tmp_str:j->file_name,1);
			if(file==NULL){
				file=bit_open("decompressed.txt",1);
			}
			
			array_init(SIZE_OF_HASH_TABLE,decompressor);//4793//10007
			
			int integrity;
			print_info(j,file_to_read,0,&integrity);
			
			if(!integrity)
				decompress(file_to_read,file,decompressor);

			else {

				char str[10];
rescan: 		
				printf("%s\n","The compressed file is corrupted, continue to extract? (y/n):\n" );
				if(!scanf("%s",str))
					goto rescan;

				printf("\n");

				switch(str[0]){
				case 'y':	decompress(file_to_read,file,decompressor);
							break;
				case 'n': _exit(-1);
							break;
				default: goto rescan;
				}
			}
			break;
		
		case 'l':
			arguments=optarg;
			init_decompressor(arguments,decompressor);
			file=bit_open(decompressor->file_to_decompress,0);
			printf("%s\n","------------------" );
			printf("Info on %s\n",decompressor->file_to_decompress );
			printf("%s\n","------------------" );
			if(file==NULL){
				file=bit_open("decompressed.txt",1);
			
			}
			info * i=getinfo(file);
			print_info(i,file,1,NULL);
			bit_close(file);
			break;

		case 's':
			compressor->d_max=(uint16_t)atoi(optarg);
			compressor->d_max&=((1UL<<15)-1);
			
			break;

		case 'h':
			printf("lz78 -o [OUTPUT_FILE] -c [FILE_TO_COMPRESS] \n");
			printf("lz78 -o [OUTPUT_DIR] -d [FILE_TO_DECOMPRESS] \n");
			printf("lz78 -l [COMPRESSED_FILE] \n");
			printf("lz77 -s [DIC_SIZE]\n");
			printf("lz78 -h help \n");
			
			break;
		
		case 'o':
			o=1;
			arguments=optarg;
			char str[50];
			strcpy(str,arguments);
			decompressor->output_file=compressor->output_file=str;
			
			
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
