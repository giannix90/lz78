
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
#include <stdlib.h>
#include <sys/types.h>

#include "lz78.h"


// this pair function return a univoque number for a pair of two value
/*
int pair_fun(int x, int y){
	int res=(x*x+3*x+2*x*y+y+y*y)/2;
	//printf("Pair %d\n",res);
	return res;
}
*/

//this hash function return the index row for a pair <num,index>
uint64_t 
hash(uint64_t num, char c)
{
	int prime_1=101;
	int prime_2=17;
	
	//int ris=(num*prime_1+c*prime_2)%hash_table_size;//hash with modulus
	uint64_t ris=((num<<7)+c)%hash_table_size;
	if(hash_table[ris].key.c!=c && hash_table[ris].key.c>='a' && hash_table[ris].key.c<= 'z' )printf("Collision in hash table!!!!!!!!!!!!!!!\n");//if i try to write where the cell is full, then i have a collision
	return ris; //i return the index of hash table
}


//this function fill the tree for compressor with all the charapter
int 
hash_fill(lz78_compressor* in)
{ 
	char c=0x09;//start from tab
	while(c<=0x7E){	//until ~
		hash_table[hash(0,c)].child_index=(c-0x09)+1; //the 1° node is reserved to the root node
		(hash_table[hash(0,c)].key).father_num=0;
		(hash_table[hash(0,c)].key).c=c;
		c++;	
	}
	
	
	in->counter_child_tree=(c-0x09); //counter tree's node for the next child
	return 0;
}


//this function allocate the hash structure
int 
hash_init(uint64_t size,lz78_compressor* in)
{
	hash_table_size=size;
	hash_table=malloc(sizeof(hash_elem)*size);
	in->hash_table_pointer=hash_table;
	if(hash_table==NULL) return -1;
	if(hash_fill(in))return -1;
	return 0;
}

void print_hash_table(){
	printf("     HASH	\n");
	printf("-------------\n");
	for(int i=0;i<hash_table_size;i++){
		for(int j=0;j<hash_table_size;j++){
			if(hash_table[j].key.father_num==i)printf("| %d | %c | %d |\n",hash_table[j].key.father_num,hash_table[j].key.c,hash_table[j].child_index);
		//printf("-------------\n");		
		}
		
	}
}

/*Initialize the lz78_compressor structure*/
void 
init_compressor(char * str_in,lz78_compressor * in)
{
	in->file_to_compress=str_in;
}


void 
init_decompressor(char * str_in,lz78_decompressor * in)
{	//I initialize the decompressor structure
	in->file_to_decompress=str_in;
}

//this function return 0 if the node is present, 1 otherwise, in this case this function add the node 
int 
look_up_and_add(int num,char c,lz78_compressor * in)
{
	if((hash_table[hash(num,c)]).child_index!=0){
		printf("Look_up: %c\n",c);
		return 0;
	}

	//add the node
	hash_table[hash(num,c)].child_index=in->counter_child_tree;
	hash_table[hash(num,c)].key.father_num=num;
	hash_table[hash(num,c)].key.c=c;
	in->counter_child_tree++;
	return 1;
}



/*------*/
//this function fill the initialize structure for the decompressor tree
void 
array_fill(lz78_decompressor * in)
{
	char c=0x09;
	while(c<=0x7E){	
		
		array_tree[(int)(c-0x09)].father_num=0;
		array_tree[(int)(c-0x09)].c=c;
		c++;	
	}

	in->counter_child_tree=(int)(c-0x09);
	//printf("Num: %d\n",in->counter_child_tree);
}

//i have to allocate the array structure for decompressor tree
void 
array_init(uint64_t size,lz78_decompressor * in)
{
	array_tree=malloc(sizeof(array_decompressor_element)*size);
	array_fill(in);
}



/*--Function for compress file--*/
void 
compress(char * str_in,lz78_compressor * in, struct bitio* file)
{
	char* buf=malloc(16); 
	uint64_t inp=0;
	int r=0;
	int father=0;
	int child=0;	
	int go=1;
	uint64_t c=0;
	int flag=0;
	int ret=0;

	struct bitio*file_out=bit_open(in->output_file,1);

	if(flag==0){
		/*Only for the first character*/
		ret=bit_read(file,(u_int)8,&inp);
		buf[1]=(char)(inp&((1UL<<8)-1));
		printf("Ric: %X\n",buf[1] & 0xff);
	}

	printf("-----------\n");
	while(go==1){
		

		while(look_up_and_add(father,buf[1],in)==0){//repeat the operation until i don't fint a node =>scroll the tree

			buf[0]=buf[1];		
			father=hash_table[hash(child,(char)(inp&((1UL<<8)-1)))].child_index;
			printf("Padre: %d\n",father);
			ret=bit_read(file,(u_int)8,&inp);	
			buf[1]=(char)(inp&((1UL<<8)-1));
			printf("Ric: %X\n",buf[1] & 0xff);
			child=father;
		}

		//printf("Output: %X Char %c\n",father & 0xff,buf[1]);
		bit_write(file_out,16, (int) father);
		father=0; 	// restart from the root
		child=0;
		c++;
		if(ret==0)break;
		
	
	}

	/*After compress i put the info of the file at the beginning of the file*/
	info* ret_info=malloc(sizeof(info));

	/*Put the info at the beginning of the file and return a structure with the info*/
	//addinfo(file_out,in->file_to_compress,0);
		
	//close the file descriptor
	bit_close(file_out);
}

char 
look_up_array(int index)
{
	if(array_tree[index].father_num==0){ 
		return(char)array_tree[index].c;
	}
		
	return -1;
}


/*--Function for Decompressor--*/
void 
decompress(struct bitio* file_to_read,struct bitio* file,lz78_decompressor* in)
{
			//char * out=malloc(sizeof(char)*150);
			char out[50];			
			out[0]='\0';
			int i=0;
			int j=0;
			char first_char=0;
			file_to_read=bit_open(in->file_to_decompress,0);
			if(file_to_read==NULL){
				printf("File not found or you don't have enough privilege to open a file\n");
				return;
			}
			uint64_t inp=0;
			int temp=0;	
			char prec_char=0;
			int first_father;
			int ret=0;
			int x=0;		

			//fseek(file_to_read->f,4,SEEK_SET);

			while(x>=0){
			
				ret=bit_read(file_to_read,16,&inp);
				first_father=temp=(int)(inp&((1UL<<16)-1))-1;

				if(first_father<=0)goto fine_file;
				
				while((first_char=look_up_array(temp))==-1){

					out[i]=(char)array_tree[temp].c;
					temp=(int)array_tree[temp].father_num;
					i++;
							
				}
				
				if(ret==0) {
					fine_file:printf("flush\n");
					//no more data to read
					flush_out_buffer(file);
					break;
				}
				
				out[i]=first_char;

				
				if(x==0){
					prec_char=(int)(inp&((1UL<<16)-1))-1;
					printf("ricevuto :%d Inserisco il nodo %d con padre %d\n",first_father,(int)in->counter_child_tree-1,prec_char);
					array_tree[(int)in->counter_child_tree-1].father_num=(int)((inp&((1UL<<16)-1))-1);
					in->counter_child_tree++;
				}
				if(x>0){
					printf("Inserisco %c nel nodo %d temp:%d\n",array_tree[temp].c,(int)in->counter_child_tree-2,temp);
					array_tree[(int)in->counter_child_tree-2].c=array_tree[temp].c;
					printf("Inserisco il nodo %d con padre %d\n",(int)in->counter_child_tree-1,(int)(inp&((1UL<<16)-1))-1);
					/*I have to add a new node*/
					array_tree[(int)in->counter_child_tree-1].father_num=(int)(inp&((1UL<<16)-1))-1;

					out[0]=((int)in->counter_child_tree-2==(int)((inp&((1UL<<16)-1))-1))?array_tree[temp].c:(char)array_tree[(int)((inp&((1UL<<16)-1))-1)].c; //If i have received the last node insert, i have to update the output string becouse i didn't have the char of the node received

					in->counter_child_tree++;
					}
				
				//out[i+1]='\0';
				for(j=i;j>=0;j--){bit_write(file,8, out[j]);				
				printf("%c",(out[j]));} // I print the buffer string
				printf("\n"); 
				i=0;
				x++;
				
				out[0]='\0';
				
			}	
			/*rewind(file_to_read->f);
			uint64_t r;
			bit_read(file_to_read,8,&r);
			printf("%c aad\n",(char)r&((1UL<<8)-1));
			bit_read(file_to_read,8,&r);
			printf("%c aad\n",(char)r&((1UL<<8)-1));
			bit_read(file_to_read,8,&r);
			printf("%c aad\n",(char)r&((1UL<<8)-1));
			bit_read(file_to_read,8,&r);
			printf("%c aad\n",(char)r&((1UL<<8)-1));
			*/
			bit_close(file_to_read);
}



