
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

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include "info.h"

#define SIZE_OF_HASH_TABLE 78644
#define MY_EOF 257

/*
 *	78644 == dictionary_size + 20% =>open hash
 *	6700417 maximum_size for hash_table => no collision => no open hash =>max velocity
*/

#if 0

typedef struct _lz78_compressor lz78_compressor;

typedef struct _lz78_decompressor lz78_decompressor;

typedef struct _array_decompressor_element array_decompressor_element;

typedef struct _hash_key hash_key;

typedef struct _hash_elem hash_elem;
#endif


typedef struct lz78_compressor{
	uint32_t d_max;
	char * file_to_compress;
	char* output_file;
	uint32_t counter_child_tree; 
	struct hash_elem * hash_table_pointer;
	uint32_t number_of_code; //number of value (number) inserted into the file
} lz78_compressor;


typedef struct lz78_decompressor{
	uint32_t d_max;
	char * file_to_decompress;
	char* output_file;
	uint32_t counter_child_tree; 
	uint32_t number_of_code;
	struct array_decompressor_element* decompressor_tree_pointer;
	info * info_ptr;
} lz78_decompressor;



typedef struct array_decompressor_element{
	uint8_t c;
	int father_num;
} array_decompressor_element;



typedef struct hash_key{
	int father_num;
	uint8_t c;
} hash_key;



typedef struct hash_elem{
	hash_key key;
	uint32_t child_index;
	int filled; //this number is a flag that signal if the elem is occupied
 	struct hash_elem * next; //used for open hash
} hash_elem;


int hash_table_size;


hash_elem * hash_table;	//compressor tree


array_decompressor_element* array_tree; // decompressor tree



void compress(lz78_compressor * in, struct bitio* file);

void decompress(lz78_decompressor* in,struct bitio* file_to_read,struct bitio* file);

void init_compressor(lz78_compressor * in,char * str_in);

void init_decompressor(lz78_decompressor * in,char * str_in);

uint64_t hash(uint64_t num, uint8_t c);

int hash_init(struct lz78_compressor* in,uint64_t size);

void array_init(struct lz78_decompressor* in,uint64_t size);

void print_hash_table();
