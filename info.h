
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

#include <time.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <openssl/sha.h>
#include "bitio.h"



typedef struct info
{
	char alg_type[4];
	uint16_t dictionary_size;
	uint16_t symbol_size;
	char file_name[40];
	off_t original_size; //32bit
	time_t time;	//32bit
	unsigned char sha1[SHA_DIGEST_LENGTH];
	uint32_t number_of_symbols; // number of symbols iserted in the file, used for SHA1

}info;

info * addinfo(struct bitio * file,struct bitio * file_to_compress ,char * name_of_file,long file_position,uint16_t size_dictionary);
info * getinfo(struct bitio * file);
unsigned char * getSHA1(unsigned long position,int size,struct bitio * file,info* in);
