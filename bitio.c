
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
#include <math.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include "bitio.h"



struct bitio* bit_open(const char* name,u_int mode)
{
	struct bitio* b;
	
	
	if(name==NULL || name[0]=='\0' || mode>1){
		errno=EINVAL;
		return NULL;			
	}

	b=calloc(1,sizeof(struct bitio));
	if(b==NULL){
		//ERROR for calloc
		errno=ENOMEM;
		return NULL;	
	}

	b->f=fopen(name,(mode==0)?"r":"w");
	
	if(b->f==NULL){
		errno=EOF;	
		free(b);
		return NULL;	
	}
	b->mode=mode;
	b->wp=0;
	b->rp=0;
	return b;
}

int bit_close(struct bitio* b)
{
	
	int ret=0;	
	if(b==NULL){
		errno=EINVAL;
		return -1;	
	}

	if(b->mode==1 && b->wp>0){
		if(fwrite((void*)&b->data,((b->wp)+7)/8,1,b->f)<=0){
			ret=-1;		
		}	
	}
	fclose(b->f);
	bzero(b,sizeof(*b));
	free(b);
	return ret;
}

int bit_write(struct bitio * b, u_int size, uint64_t data)
{

	/**
	The C library function size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) 
	writes data from the array pointed to, by ptr to the given stream.
	
	*ptr − This is the pointer to the array of elements to be written.

	*size − This is the size in bytes of each element to be written.

	*nmemb − This is the number of elements, each one with a size of size bytes.

	*stream − This is the pointer to a FILE object that specifies an output stream.
	
	**/
	//printf("Data da stampare1: %d\n",(int)data&(1UL << 7)-1);
	int space;
	if(b==NULL || b-> mode!=1 || size > 64){
		errno=EINVAL;
		return -1;	
	}

	if(size==0){
		//no bit to copy
		return 0;	
	}
	data&=(1UL << size)-1;

	space=64-(b->wp); // space available in the buffer
	if(size<=space){
		//we have to take the block, shift by wp position and then copy the block into the buffer
			//1UL is 1 unsigned long
		
		b->data|=data<<b->wp;
		b->wp+=size;
		//data&=~data;
	}
	else{
		//printf("Data da stampare: %d\n",(int)b->data&(1UL << 8)-1);
		/*Insert part of data into buffer*/

		b->data |= (data&((1UL<<space)-1)) << b->wp;
		/*----*/

		if(fwrite((void*)&b->data,1,8,b->f)<=0){
			errno=ENOSPC;
			
			return -1;		
		}
			
		b->data=data>>space;
		b->wp=size-space;
		
	}
	return 0;
}


int bit_read(struct bitio* b,u_int size,uint64_t *data)
{
		
		/*	<-----space----->
		 --------------------------------
		||	|		|	||	==> b->data
		 --------------------------------
  			^		^
			|(wp)		|(rp)
		*/

	if(b==NULL || b-> mode!=0 ||size>64){
		errno=EINVAL;
		return -1;
	}
	//at the beginning we want clear all bit of data buffer
	*data=0;

	int space=(int)(b->wp)-(int)(b->rp);

	if(size==0){
		return 0; // no mre bit to read	
	}
	
	if(size<=space){
		// in this case we want to read a smaller number of bit than the all unreaded  

		*data=(b->data>>b->rp)&((1UL<<size)-1);
		b->rp+=size;
		return size;	
	}
	
	else{
		// in this case we want to read more number of bit than the available unreaded bit
		*data=(b->data>>(b->rp)); //i don't need to use a mask because i want read all unreaded	bit

		/*
		The C library function size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) 
		reads data from the given stream into the array pointed to, by ptr.		
		
		*ptr − This is the pointer to a block of memory with a minimum size of size*nmemb bytes.

		*size − This is the size in bytes of each element to be read.

		*nmemb − This is the number of elements, each one with a size of size bytes.

		*stream − This is the pointer to a FILE object that specifies an input stream.
		
		*/

		int ret=fread(&(b->data),1,8,b->f);
		
		
		b->wp=ret*8;
		if(b->wp>=size-space){
			/*space is the number of bit readed; size-space is the remaining number of bit to read: 
				-if b->wp >= size-space means that we can read the remain bit and we can add this to data buffer
				-else we need to handle the situation					
			*/
			*data^=*data; //I clear all bit of *data buffer (*data XOR *data)
			
			*data |= b->data<<space;
			
			if(size<64)
				*data &= (uint64_t)((1UL<<size)-1); //i have to mask the bit that we want use in the buffer data (only if size<64, otherwise i go out of UL and i mask all)

			b->rp=size-space;
			
			return size-space;
		}
		else{
			*data |= b->data<<space;
			*data &= (1UL<<(space+b->wp))-1;
			b->rp=b->wp;//i've read all buffer
			return space+b->wp;		
		}
	}
}

int 
flush_out_buffer(struct bitio* b)
{
	int space;
	space=64-(b->wp);
	//printf("devo riempire altri %d\n",space);
	if(space==64)
		return 0;//buffer empty
	else{
		//i need to flush the buffer
		int ret=fwrite((void*)&b->data,((b->wp)+7)/8,1,b->f);
		b->wp=0;
		b->data^=b->data;
		if(ret<=0)errno=ENOSPC;
		return ret;
	}
	
}
