#include "code.h"
#include "endian.h"
#include "io.h"
#include "trie.h"
#include "word.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define OPTIONS "vi:o:"

// returns the number of bits required to represent "code"
int bit_length(uint16_t code) {
    int length = 0;
    do {
        length += 1;
        code >>= 1;
    } while (code > 0);
    return length;
}
// returns the number of bytes required to represent "bits"
uint64_t Bytes(uint64_t bits) {
    if (bits % 8 == 0) {
        return (bits / 8);
    } else {
        return (bits / 8 + 1);
    }
}

int main(int argc, char **argv) {
    int infile = STDIN_FILENO;
    int outfile = STDOUT_FILENO;
    int opt = 0;
    bool stats = false;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'v':
            // will print out the statistics of decompressed file
            stats = true;
            break;
        case 'i':
            // specifies the infile, exits program if file doesn't exist
            infile = open(optarg, O_RDONLY);
            if (infile == -1) {
                fprintf(stderr, "Failed to open file.\n");
                return 1;
            }
            break;
        case 'o':
            // specifies the outfile, creates a file if doesn't already exist
            outfile = open(optarg, O_WRONLY | O_CREAT | O_TRUNC, 0600);
            if (outfile == -1) {
                fprintf(stderr, "Failed to open file.\n");
                return 1;
            }
            break;
        }
    }

    struct stat sb;
    fstat(infile, &sb);
    // reads in the header
    FileHeader header = { 0, 0 };
    read_header(infile, &header);
    // if system is big endian, the magic number and protection bits have to be swapped
    if (big_endian()) {
        header.magic = swap32(header.magic);
        header.protection = swap16(header.protection);
    }
    // if the magic number of the file is not as specified it cannot be decompressed
    // using this program
    if (header.magic != 0xBAADBAAC) {
        fprintf(stderr, "Bad magic number.\n");
        return 1;
    }
    fchmod(outfile, header.protection);
    // This is the LZ78 algorithm for decompression
    //  It reads in the pairs and uses the word table to output them to symbols
    WordTable *table = wt_create();
    uint8_t curr_sym = 0;
    uint16_t curr_code = 0;
    uint16_t next_code = START_CODE;
    while (read_pair(infile, &curr_code, &curr_sym, bit_length(next_code)) == true) {
        table[next_code] = word_append_sym(table[curr_code], curr_sym);
        write_word(outfile, table[next_code]);
        next_code = next_code + 1;
        if (next_code == MAX_CODE) {
            wt_reset(table);
            next_code = START_CODE;
        }
    }
    flush_words(outfile);
    // if the v command line option was specified, print out the stats
    if (stats == true) {
        printf("Compressed file size: %lu bytes\n", Bytes(total_bits));
        printf("Uncompressed file size: %lu bytes\n", total_syms);
        float total_bytes = Bytes(total_bits);
        float saving = 100 * (1.0 - (total_bytes / total_syms));
        printf("Space saving: %4.2lf%%\n", saving);
    }

    wt_delete(table);
    close(infile);
    close(outfile);
    return 0;
}
