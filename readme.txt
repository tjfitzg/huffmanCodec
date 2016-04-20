The purpose of this program is to provide the user with the ability to encode and decode any file using the lossless huffman encoding algorithm. The executable can be created by running "make" in a bash script or by running the following in a bash script...

gcc -g huff.c -o huff -Wall -lm

running "make clean" will remove the executable

The usage of this program is as follows...

Usage:

Compress:
./huff [-c] [saved filename]
     
or

Decompress:
./huff [-u] [compressed filename] [desired uncompressed filename]


A compressed file will be saved in the current directory with .huff appended to the filename.

Once a file has been decompressed it can be seen that the huffman encoding algorithm worked properly
and was indedd lossless by running the following command...
diff [file1name] [file2name]

The command...
ls -al
Can be used to view the size of the files in bytes to view compression size vs decompressed

A story.txt file has been included for compression and decompression but use any file you would like
