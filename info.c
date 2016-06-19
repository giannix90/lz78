
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
	out_info->alg_type[0]=0x4C;	// L
	out_info->alg_type[1]=0x5A;	// Z
	out_info->alg_type[2]=0x37;	// 7
	out_info->alg_type[3]=0x38;	// 8

	out_info->dictionary_size=(uint16_t) size_dictionary;
	//printf("dictionary: %d\n",out_info->dictionary_size );

	out_info->symbol_size=16;

	//Insert the file name
	/*
	if(strlen(strncpy(out_info->file_name,name_of_file,40))<=1){
		printf("%s\n", "Invalid input file name");
		strcpy(out_info->file_name,"Undefined.txt");
	}*/
	char* t=strrchr(name_of_file,'/');
	if(t)
		strncpy(out_info->file_name,t+1,strlen(t+1));
	else
		strncpy(out_info->file_name,name_of_file,strlen(out_info->file_name));
//----------------------

	//Insert the original size
	int fd = fileno(file_to_compress->f); // get fd from FILE * pointer
	struct stat info_on_file;
	if(fstat(fd,&info_on_file))
			printf("Error in readinf file stat\n");
	out_info->original_size=info_on_file.st_size;


	//insert the date-time
	out_info->time=time(NULL);

	if(file_position==0)rewind(file->f); //put the byte pointer at the beginning of the file
			
		
	for(int i=0;i<4;i++){

		bit_write(file,(u_int)8,(char)out_info->alg_type[i]);
			
	}
			
	bit_write(file,(u_int)16,(uint16_t)out_info->dictionary_size); //i put the size of dictionary (65536 by defaults)

	bit_write(file,(u_int)16,(int)out_info->symbol_size); //size of the symbol


	//I put the file name in the info
	for(int i=0;i<40;i++){

		bit_write(file,(u_int)8,out_info->file_name[i]);
			
	}

	
	//I put the original size of the file
	bit_write(file,(u_int)(sizeof(off_t)*8)/2,out_info->original_size&((1UL<<((sizeof(off_t)*8)/2))-1));
	bit_write(file,(u_int)(sizeof(off_t)*8)/2,out_info->original_size>>((sizeof(off_t)*8)/2)&((1UL<<((sizeof(off_t)*8)/2))-1));


	//I pute the time
	bit_write(file,(u_int)(sizeof(time_t)*8)/2,out_info->time&((1UL<<((sizeof(time_t)*8)/2))-1));
	bit_write(file,(u_int)(sizeof(time_t)*8)/2,out_info->time>>((sizeof(time_t)*8)/2)&((1UL<<((sizeof(time_t)*8)/2))-1));
	
	for(int i=0;i<20;i++){
		//I put the Hash SHA-1 of the compressed part
		bit_write(file,(u_int)8*sizeof(unsigned char),0xFFFFFFFF&((1UL<<8*sizeof(unsigned char))-1));
	}

	//I pute the umber of symbols iserted in the file
	bit_write(file,(u_int)32,0xFFFFFFFF);

	return out_info;
	

}


//Return the info at the beginning of the file in the decompressor side
info * 
getinfo(struct bitio * file)
{
	info * in_info=malloc(sizeof(info));

	uint64_t inp=0;

	fseek(file->f,0,SEEK_SET);

	/*get algorithm type*/
	for(int i=0;i<4;i++){
		
		bit_read(file,(u_int)8,&inp);
		in_info->alg_type[i]=(char)(inp&((1UL<<8)-1));
			
	}

	//get the dictionary size
	bit_read(file,(u_int)16,&inp);
	in_info->dictionary_size=(int)inp&((1UL<<16)-1);
	
	//get the symbol size
	bit_read(file,(u_int)16,&inp);
	in_info->symbol_size=(int)inp&((1UL<<16)-1);
	
	//get the file name
	int i=0;
	for(i=0;i<40;i++){

		bit_read(file,(u_int)8,&inp);
		in_info->file_name[i]=(char)inp&((1UL<<8)-1);	
	}
	in_info->file_name[39]='\0';
	
	//get the original size

	bit_read(file,(u_int)8*sizeof(off_t),&inp);
	in_info->original_size=inp;
		
	//get the time
	bit_read(file,(u_int)8*sizeof(time_t),&inp);
	in_info->time=inp&((1UL<<32)-1);

	char text[100];
	struct tm *t = localtime(&in_info->time);
	strftime(text, sizeof(text)-1, "%d %m %Y %H:%M", t);


	//get the checksum
	for(i=0;i<20;i++){

		bit_read(file,(u_int)8,&inp);
		in_info->sha1[i]=inp&((1UL<<8*sizeof(unsigned char))-1);
						
	}

	bit_read(file,(u_int)32,&inp);
	in_info->number_of_symbols=inp&((1UL<<32)-1);


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
		bit_read(file,(u_int)16,&inp);
		data[i]=(char)inp&((1UL<<8)-1);
		data[i+1]=(char)(inp>>8)&((1UL<<8)-1);
	}

	//unsigned char hash[SHA_DIGEST_LENGTH];
	SHA1((unsigned char*)data, (size*2)-10, in->sha1);

	return in->sha1;
}