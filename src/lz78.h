
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
//#include "bitio.h"
#include "info.h"

int hash_table_size;


typedef struct lz78_compressor{
	uint32_t d_max;
	char * file_to_compress;
	char* output_file;
	uint32_t counter_child_tree; 
	struct hash_elem * hash_table_pointer;
} lz78_compressor;

typedef struct lz78_decompressor{
	uint32_t d_max;
	char * file_to_decompress;
	char* output_file;
	uint32_t counter_child_tree; 
} lz78_decompressor;

typedef struct array_decompressor_element{
	char c;
	int father_num;
} array_decompressor_element;

typedef struct hash_key{
	int father_num;
	char c;
} hash_key;

typedef struct hash_elem{
	hash_key key;
	uint32_t child_index;
} hash_elem;

hash_elem * hash_table;	//compressor tree

array_decompressor_element* array_tree; // decompressor tree

void compress(char * str_in,lz78_compressor * in, struct bitio* file);

void decompress(struct bitio* file_to_read,struct bitio* file,lz78_decompressor* in);

void init_compressor(char * str_in,lz78_compressor * in);

void init_decompressor(char * str_in,lz78_decompressor * in);

uint64_t hash(uint64_t num, char c);

int hash_init(uint64_t size,struct lz78_compressor* in);

void array_init(uint64_t size,struct lz78_decompressor* in);

void print_hash_table();
