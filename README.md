# Lempel-Ziv Compression

    This program uses the LZ78 algorithm to compress and decompress files both in big and little
    endian computers, utilizing tries and word tables. It performs these processes on user input
    from stdin or any other spcified files.

## build

$ clang -Wall -Werror -Wextra -Wpedantic -c encode.c
$ clang -Wall -Werror -Wextra -Wpedantic -c io.c
$ clang -Wall -Werror -Wextra -Wpedantic -c trie.c
$ clang -o encode encode.o io.o trie.o
$ clang -Wall -Werror -Wextra -Wpedantic -c decode.c
$ clang -Wall -Werror -Wextra -Wpedantic -c word.c
$ clang -o decode decode.o io.o word.o
$ clang -Wall -Werror -Wextra -Wpedantic -c entropy.c
$ clang -o entropy entropy.o -lm

## Running

$ ./encode
$ ./decode
$ ./entropy

## Command Line Options

$ encode
$ -v: Print compression statistics to stderr
$ -i: Specify input to compress
$ -o: Specify output of compressed input
$ decode
$ -v: Print decompression statistics to stderr
$ -i: Specify input to decompress
$ -o: Specify output of decompressed input

## Cleaning

    $ rm -f encode *.o decode *.o entropy*.o


## Format

    $  clang-format -i -style=file io.h io.c trie.c trie.h encode.c endian.h word.h word.c decode.c entropy.c

## valgrind

$ valgrind might signal a warning but this is because of the initlization of the header
$ and it is not really an issue
