all: huff.c 
	gcc -g huff.c -o huff -Wall -lm
clean: 
	$(RM) huff
