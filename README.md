#lz78 compressor
=================

Basic Implementation of LZ78 Compression Alghorithm


##Installation

	$ Make

	$ Make install 


Examples
--------

to compress:

	$ lz78 -o [FILE_NAME].lz78 -s [DICTIONARY_SIZE] -c [INPUT_FILE]

to decompress:

	$ lz78 -o [OUTPUT_DIR] -d [COMPRESSED_INPUT_FILE]

to obtain info of the compressed file:

	$lz78 -i [COMPRESSED_INPUT_FILE]


Developer
-----------

Gianni Pollina (gianni.pollina1@gmail.com)

