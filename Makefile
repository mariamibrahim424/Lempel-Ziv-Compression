CC = clang 
CFLAGS = -Wall -Werror -Wextra -Wpedantic

all : encode decode entropy

encode: encode.o io.o trie.o 
	 $(CC) -o encode encode.o io.o trie.o 

encode.o: encode.c
	$(CC) $(CFLAGS) -c encode.c

decode: decode.o io.o word.o
	$(CC) -o decode decode.o io.o word.o

decode.o: decode.c
	$(CC) $(CFLAGS) -c decode.c

io.o: io.c                        
	$(CC) $(CFLAGS) -c io.c 

trie.o: trie.c
	$(CC) $(CFLAGS) -c trie.c

word.o: word.c
	$(CC) $(CFLAGS) -c word.c

entropy: entropy.o
	$(CC) -o entropy entropy.o -lm

entropy.o: entropy.c
	$(CC) $(CFLAGS) -c entropy.c

clean:
	rm -f encode *.o decode *.o entropy *.o   

format: 
	clang-format -i -style=file io.h io.c trie.c trie.h encode.c endian.h word.h word.c decode.c entropy.c



