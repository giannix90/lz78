
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
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>


struct bitio{
	FILE * f; //pointer to the file
	uint64_t data; // local buffer
	u_int wp; //write index pointer into the buffer
	u_int rp; //read index pointer into the buffer
	u_int mode; //0=>read  1=>write 
};



struct bitio* bit_open(const char* name,u_int mode);

int bit_close(struct bitio * b);

int bit_write(struct bitio* b, u_int size, uint64_t data);

int bit_read(struct bitio* b, u_int size, uint64_t* data);

int flush_out_buffer(struct bitio* b);

