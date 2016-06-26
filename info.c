
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

#include "info.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/*This function accept bitio structure, lz78_compressor and a file_position witch rappresents 
	the position in the file at wich put the info, the function returns a streucture's pointer witch contain the info inserted in the file*/
info * 
addinfo(struct bitio * file,struct bitio * file_to_compress ,char * name_of_file,long file_position,uint16_t size_dictionary)
{
	info * out_info=malloc(sizeof(info));

	//I insert the algorithm type
	out_info->alg_type[0]='L';	
	out_info->alg_type[1]='Z';	
	out_info->alg_type[2]='7';	
	out_info->alg_type[3]='8';	

	out_info->dictionary_size=size_dictionary;

	out_info->symbol_size=16;

	//Insert the file name
	
	char* t=strrchr(name_of_file,'/');

	if(t)
		strncpy(out_info->file_name,t+1,strlen(t+1));
	else
		strncpy(out_info->file_name,name_of_file,strlen(out_info->file_name));
//----------------------


	/*
	*	mai usare il fd dell bit open per altre operazioni
	*
	*/

	//Insert the original size

	struct stat info_on_file;

	if(stat(name_of_file,&info_on_file)) // i reopen a file with stat, without using already opened FD trough bitopen
			printf("Error in readinf file stat\n");

	out_info->original_size=info_on_file.st_size;



	//insert the date-time
	out_info->time=time(NULL);
			
		
	for(int i=0;i<4;i++){

		bit_write(file,(u_int)8,(char)out_info->alg_type[i]);
			
	}
			
	bit_write(file,(u_int)16,(uint16_t)out_info->dictionary_size); //i put the size of dictionary (65536 by defaults)

	bit_write(file,(u_int)16,(int)out_info->symbol_size); //size of the symbol


	//I put the file name in the info
	for(int i=0;i<40;i++){

		bit_write(file,8,out_info->file_name[i]);
			
	}

	
	//I put the original size of the file
	/*
	*  Format of information : <byte7,byte6,.......,byte0>
	*
	*	I use the little-endian format to store the information, and i store the information in a block of 64bit on the HD
	*
	*	-----------------------------------------
	*	|byte0	|byte1	|byte2	| ...	|byte7	|
	*	|		|		|		|		|		|
	*	-----------------------------------------
	*
	*/

	uint8_t tmp;

	for(int i=0;i<64;i+=8){
	
		tmp=(out_info->original_size>>i)&0xFF;
		bit_write(file,8,tmp);
	}



	//I pute the time
	for(int i=0;i<64;i+=8){
	
		tmp=(((uint64_t)out_info->time)>>i)&0xFF;
		bit_write(file,8,tmp);
	}
	
	for(int i=0;i<20;i++){
		//I put the Hash SHA-1 of the compressed part
		bit_write(file,8,0xFF);
	}

	//I pute the umber of symbols iserted in the file
	bit_write(file,32,0xFFFFFFFF);

	return out_info;
	

}


//Return the info at the beginning of the file in the decompressor side
info * 
getinfo(struct bitio * file)
{
	info * in_info=malloc(sizeof(info));


	uint64_t inp=0;


	/*get algorithm type*/

	for(int i=0;i<4;i++){
		
		bit_read(file,8,&inp);
		in_info->alg_type[i]=(char)(inp&0xFF);
			
	}


	//get the dictionary size
	bit_read(file,16,&inp);
	in_info->dictionary_size=(uint16_t)inp&0xFFFF;

	
	//get the symbol size
	bit_read(file,16,&inp);
	in_info->symbol_size=(uint16_t)inp&0xFFFF;

	
	//get the file name
	int i=0;
	for(i=0;i<40;i++){

		bit_read(file,8,&inp);
		in_info->file_name[i]=(char)inp&0xFF;	
	}
	in_info->file_name[39]='\0';

	
	//get the original size
	bit_read(file,64,&inp);
	in_info->original_size=inp;

		
	//get the time
	bit_read(file,64,&inp);
	in_info->time=inp;



	//get the checksum
	for(i=0;i<20;i++){

		bit_read(file,8,&inp);
		in_info->sha1[i]=inp;
						
	}

	bit_read(file,32,&inp);
	in_info->number_of_symbols=inp&0xFFFFFFFF;


	return in_info;
}



unsigned char *
getSHA1(unsigned long position,int size,struct bitio* file,info * in){

	char data[size*2];
	uint64_t inp=0;
	
	//i need to reset data array
	for(int i=0;i<size*2;i++){
		
		data[i]^=data[i]; // XOR
		
	}

	fseek(file->f,position,SEEK_SET);


	for(int i=0;i<size;i+=2){
		bit_read(file,16,&inp);
		data[i]=(char)inp&0xFF;
		data[i+1]=(char)(inp>>8)&0xFF;
	}

	//unsigned char hash[SHA_DIGEST_LENGTH];
	SHA1((unsigned char*)data, (size*2)-10, in->sha1);

	return in->sha1;
}
