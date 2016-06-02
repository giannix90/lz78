
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



/*This function accept bitio structure, lz78_compressor and a file_position witch rappresents 
	the position in the file at wich put the info, the function returns a streucture's pointer witch contain the info inserted in the file*/
info * 
addinfo(struct bitio * file,char * name_of_file,long file_position)
{
	info * out_info=malloc(sizeof(info));

	//I insert the algorithm type
	out_info->alg_type[0]='l';
	out_info->alg_type[0]='z';
	out_info->alg_type[0]='7';
	out_info->alg_type[0]='8';

	out_info->dictionary_size=99991;

	out_info->symbol_size=16;

	//Insert the file name
	if(strlen(strncpy(out_info->file_name,name_of_file,40))<=1){
		printf("%s\n", "Invalid input file name");
		strcpy(out_info->file_name,"Undefined.txt");
	}

	//Insert the original size
	int fd = fileno(file->f); // get fd from FILE * pointer
	struct stat info_on_file;
	fstat(fd,&info_on_file);

	out_info->original_size=info_on_file.st_size;

	//insert the date-time
	out_info->time=time(NULL);

	if(file_position==0)rewind(file->f); //put the byte pointer at the beginning of the file
			
		
	for(int i=0;i<5;i++){

		bit_write(file,(u_int)8,out_info->alg_type[i]);
			
	}
			
			
	flush_out_buffer(file);
	



}