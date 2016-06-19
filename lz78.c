
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
#include <openssl/sha.h>

#include "lz78.h"


void 
insert_tail_open_hash(hash_elem *pl,uint64_t num, uint8_t c,uint32_t child_index)
{

	hash_elem* t;

			/* empty list */
  	if(pl==NULL) {

    	pl=malloc(sizeof(struct hash_elem));
    	(pl)->child_index=child_index;
    	(pl)->key.father_num=num;
    	(pl)->key.c=c;
    	(pl)->filled=1;
    	(pl)->next=NULL;
    	return;
  }

  			/* at least one element */
  	t=pl;

		/* scroll until the end of list */
  	while(t->next!=NULL)
    	
    	t=t->next;

	/* insert the element */
  	t->next=malloc(sizeof(struct hash_elem));
  	t=t->next;


	/* update the element*/
  	t->child_index=child_index;
  	t->key.father_num=num;
  	t->key.c=c;
  	t->filled=1;
  	t->next=NULL;

}



//this function insert a value inside hash table
int
hash_insert(uint64_t num, uint8_t c,uint32_t child_index,lz78_compressor* in)
{

	uint64_t hs=hash(num,c);
	
	if(in->hash_table_pointer[hs].filled==1){

		/*This means:
				
				-If the filled flag is setted: then we have a collision in hash table
			*/

		/*Hash row is already accupied (COLLISION)*/
		insert_tail_open_hash(&in->hash_table_pointer[hs],num,c,child_index); // we insert the elem in tail
		in->counter_child_tree++;
		return -1;

	}
	else{
		
		//Empty element case: i add in the simple hash table
		in->hash_table_pointer[hs].child_index=child_index;
		in->hash_table_pointer[hs].key.father_num=num;
		in->hash_table_pointer[hs].key.c=c;
		in->hash_table_pointer[hs].filled=1;
		in->counter_child_tree++;
		return 0;
	}	
}


/*
*
*	hash_lookup return the pointer to the hash_elem if found it, NULL otherwise
*
*/

hash_elem *
hash_lookup(hash_elem * hash_table,uint64_t num,uint8_t c)
{
	/*This function controll the vector of elem in row_index and return the pointer to the rigth elem*/
	uint64_t hs=hash(num,c);

		//simple list scroll
		hash_elem* t;
		t=&hash_table[hs];
		
		while(t!=NULL){

			if(t->key.father_num==num && t->key.c==c)
				
				//in this case i've found the element
				return t;

			t=t->next;
		}
	return NULL;
}



//this hash function return the index row of hash table for a pair <num,index>
uint64_t 
hash(uint64_t num, uint8_t c)
{
	
	int ris=((num<<8)+c)%SIZE_OF_HASH_TABLE;

	return ris; //i return the index of hash table
}



void 
_free_list(hash_elem *l)
{
   if (l != NULL) { 
    _free_list(l->next);
    free(l);
   }
}



void
open_hash_reset(lz78_compressor* in)
{
	for(uint32_t i=0;i<SIZE_OF_HASH_TABLE;i++){

		if(in->hash_table_pointer[i].next!=NULL){

			_free_list(in->hash_table_pointer[i].next);

		}
	}
}



//this function fill the tree for compressor with all the charapter
int 
hash_fill(lz78_compressor* in)
{ 
	int x=0;
	uint8_t c=0x00;//start from 0x00 value

	//0 is reserved for the root

	while(x<=255){	//until 0xFF
			hash_insert(0,(uint8_t)c,(uint32_t)x+1,in);

		c++;
		x++;	
	}
	
	//257  is reserved for EOF

	in->counter_child_tree=(257)+1; //counter tree's node for the next child
	return 0;
}



//this function allocate the hash structure
int 
hash_init(uint64_t size,lz78_compressor* in)
{

	hash_table_size=size;
	hash_table=(hash_elem*)calloc(1,sizeof(hash_elem)*size);
	in->hash_table_pointer=hash_table;

	if(hash_table==NULL) 
		return -1;

	if(hash_fill(in))
		return -1;

	return 0;
}



#if 0

void 
print_hash_table()
{
	printf("     HASH	\n");
	printf("-------------\n");
	for(int i=0;i<hash_table_size;i++){
		for(int j=0;j<hash_table_size;j++){
			if(hash_table[j].key.father_num==i)
				printf("| %d | %c | %d |\n",hash_table[j].key.father_num,hash_table[j].key.c,hash_table[j].child_index);
		//printf("-------------\n");		
		}
		
	}
}

#endif



/*Initialize the lz78_compressor structure*/
void 
init_compressor(char * str_in,lz78_compressor * in)
{
	in->file_to_compress=str_in;
}



void 
init_decompressor(char * str_in,lz78_decompressor * in)
{	
	//I initialize the decompressor structure
	in->file_to_decompress=str_in;

}



//this function return the number of child if the node is present, 0 otherwise, in this case this function add the node 
uint32_t 
look_up_and_add(int num,uint8_t c,lz78_compressor * in)
{

	hash_elem* child=hash_lookup(in->hash_table_pointer,num,c);

	if(child==NULL){
		/*
		        -If i enter in this statement this means that don't exitst this child, then i have to insert it and return 0

		*/

		//add the new node to the branch
		hash_insert(num,c,in->counter_child_tree,in);
		return 0;
	}

		//the cildren with father num and value c exists, then i have to return the child_index
		return (uint32_t)child->child_index;	
	
}



/*------*/
//this function fill the initialize structure for the decompressor tree
void 
array_fill(lz78_decompressor * in)
{
	int x=0;
	uint8_t c=0x00;
	while(x<=255){	
		
		in->decompressor_tree_pointer[(uint8_t)x+1].father_num=0;
		in->decompressor_tree_pointer[(uint8_t)x+1].c=c;
		c++;	
		x++;
	}
	//257 reserved for EOF
	in->counter_child_tree=(int)(257)+1;
	
}



//i have to allocate the array structure for decompressor tree
void 
array_init(uint64_t size,lz78_decompressor * in)
{
	array_tree=malloc(sizeof(array_decompressor_element)*size);
	in->decompressor_tree_pointer=array_tree;
	array_fill(in);
}



/*
	-This function return the value of the node, the special value 0x0100 when i reach the upper level of node attached to the root,
		this special flag signal the end of the branch
*/
uint16_t 
look_up_array(int index,lz78_decompressor * in)
{
	if(in->decompressor_tree_pointer[index].father_num==0){ 

		//when i reach the upper level (node attached to the root) i set the flag 0x0100 and return the value 0x<01,father_num>
		//return 0x0100;
		return (uint16_t)((in->decompressor_tree_pointer[index].c&0x00FF)|0x0100);		
	}

	// i return uint16_t becouse in the most significant bit i return the flag wich signal the root	
	
	return(uint16_t)in->decompressor_tree_pointer[index].c&0x00FF; //normally return the value of the node
}



array_decompressor_element*
get_elem_array_tree(int index,lz78_decompressor * in)
{
	return &in->decompressor_tree_pointer[index];
}



int
insert_into_array(int index,lz78_decompressor * in,uint8_t *cha,int *father)
{

	if((void*)cha != NULL){
		get_elem_array_tree(index,in)->c=*cha;
	}

	if((void*)father != NULL){
		get_elem_array_tree(index,in)->father_num=*father;	
	}

	if (((void*)cha )==NULL && ((void*)father) == NULL)
		return 0;

	return 1;
	
}



/*--Function for compress file--*/
void 
compress(char * str_in,lz78_compressor * in, struct bitio* file)
{
	char buf;
	uint64_t inp=0;
	int father=0;	
	int ret=0;
	int child=0;

	struct bitio*file_out=bit_open(in->output_file,1);


	/*After compress i put the info of the file at the beginning of the file*/
	info* ret_info=malloc(sizeof(info));


	/*Put the info at the beginning of the file and return a structure with the info*/
	ret_info=addinfo(file_out,file,in->file_to_compress,0,in->d_max);
	in->number_of_code^=in->number_of_code; //XOR


	reset:

	
		/*Only for the first character*/
		ret=bit_read(file,(u_int)8,&inp);
		buf=(uint8_t)(inp&((1UL<<8)-1));

	


	while(1){
		
/*Scroll the branch of the tree and update it at the end*/

		while((child=look_up_and_add(child,buf,in))!=0){

			//repeat the operation until i don't fint a node =>scroll the tree
			father=child;
			ret=bit_read(file,(u_int)8,&inp);	//read a new char
			buf=(uint8_t)(inp&((1UL<<8)-1));

		}
/*-----------------------------*/


		bit_write(file_out,16, (int) father); //output symbol

		in->number_of_code++; //increase number of symbols putted on the file

		child=0;// restart from the root
	
		if(ret==0) //No char to fetch from input file
				break;



/*Control for full dictionary*/	
		if(in->counter_child_tree>=in->d_max-1){

			//I the dictionary reach the max size i need to empty the tree and restart the compressio from the current position

			open_hash_reset(in);
			memset(in->hash_table_pointer,0,sizeof(hash_elem)*SIZE_OF_HASH_TABLE); //reset the hash table structure
			in->counter_child_tree=0;
			hash_fill(in);
			
			goto reset; //restart the compression

		};	
/*---------------------------*/	


	}


/*---EOF---*/
	bit_write(file_out,16, 257); //put the EOF (Node 257 of tree)
	in->number_of_code++;
	flush_out_buffer(file_out);
/*---------*/


/*Function for the integrity of the file*/

	//add the SHA-1 of the compressed-file in the header
	long old_index=ftell(file_out->f);//save the curren pointer to position of the file
	
	struct bitio* file_out_rd=bit_open(in->output_file,0);//I re-open the output file but in in read mode this time
	
	getSHA1(88,in->number_of_code,file_out_rd,ret_info); //I do the SHA-1 over the whole file
	
	bit_close(file_out_rd);

	fseek(file_out->f,(long)0x40,SEEK_SET); //I write the SHA-1 hash in the correct position

	for (int i = 0; i < 20; i ++) {
		bit_write(file_out,(u_int)8*sizeof(unsigned char),(unsigned char)ret_info->sha1[i]);
    }

	bit_write(file_out,(u_int)32,in->number_of_code);

    flush_out_buffer(file_out);

	fseek(file_out->f,old_index,SEEK_SET); // restore the pointer of the file in order to close the file in the rigth way

/*end of function for the integrity*/




	//close the file descriptor
	bit_close(file_out);
}



/*--Function for Decompressor--*/
void 
decompress(struct bitio* file_to_read,struct bitio* file,lz78_decompressor* in)
{
			
			uint8_t out[65536];	//max size of a branch equal to size of dictionary		
			int i=0;
			int j=0;
			uint16_t first_char=0;

			file_to_read=bit_open(in->file_to_decompress,0);

			if(file_to_read==NULL){
				
				printf("File not found or you don't have enough privilege to open a file\n");
				
				return;
			}

			uint64_t inp=0;
			int temp=0;	
			int ret=0;
			int x=0;
			int father_num=0;
			uint8_t previous_value_of_node=0;		


			in->info_ptr=getinfo(file_to_read);

			reset:
			while(x>=0){
			

				//Read an input symbol
				ret=bit_read(file_to_read,16,&inp);
				father_num=temp=(int)(inp&((1UL<<16)-1));


				if(temp==257)
					goto fine_file; // i received the EOF symbol



				if(ret<=0) {
					//no more data to read
	fine_file:		printf("The file was correctly extracted...\n");
					flush_out_buffer(file);
					break;
				}


/*Obtain a branch of the tree*/
				
				while(!((first_char=look_up_array(temp,in))&(uint16_t)0x0100)){ 

					//rise up the tree until find the root (return the value with 0x0100 flag if i have reach the root)

					out[i]=first_char&((1UL<<8)-1); // fill the local buffer out 
					temp=(int)get_elem_array_tree(temp,in)->father_num; //rise on the father
					i++;
							
				}
				

				out[i]=first_char&((1UL<<8)-1);; // add the last char of the branch becouse the loop exit before add it to buffer out
/*--------------------------*/
				


/*Update the tree*/
			

				if(x>0){
					
					//(Only from the second symbol)

					//Insert the value of the previous node (Only from the second symbol)
					previous_value_of_node=(uint8_t)get_elem_array_tree(temp,in)->c;
					insert_into_array((int)in->counter_child_tree-1,in,&previous_value_of_node,NULL);

			
					//If i have received the last node insert, i have to update the output string becouse i didn't have the char of the node received
					out[0]=((int)in->counter_child_tree-1==father_num)?(uint8_t)get_elem_array_tree(temp,in)->c:(uint8_t)get_elem_array_tree(father_num,in)->c; 
					
				}

					/*I have to add a new node*/
					insert_into_array((int)in->counter_child_tree,in,NULL,&father_num);
					in->counter_child_tree++;
				
/*----------------*/



/*Flush out in the file the branch of the tree */					
				for(j=i;j>=0;j--){
					bit_write(file,8, out[j]);				
				} 
/*------------------------*/


				i=0;

				if(x==0) //is not necessary increase every time the x, only to distinguish the first symbol readed
					x++;
				

/*Control for full dictionary*/
				if(in->counter_child_tree>=in->d_max-1){
					//need to empty the array based tree for the decompressor
					memset(in->decompressor_tree_pointer,0,sizeof(array_decompressor_element)*SIZE_OF_HASH_TABLE);
					in->counter_child_tree=0;
					array_fill(in);
					x=0;
					goto reset;
				}
/*--------------------------*/				
				
			}	
			
			bit_close(file_to_read);
}


/*-------End of File------*/