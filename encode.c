#include "code.h"
#include "io.h"
#include "trie.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
            // will print out the statistics of compressed file
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
            // specifies the outfile, creates file if doesn't already exist
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

    // create a file header and set the magic number as well as the file protection
    FileHeader header = { 0, 0 };
    header.magic = MAGIC;
    header.protection = sb.st_mode;
    fchmod(outfile, header.protection);
    write_header(outfile, &header);

    // LZ78 Algorithm
    // It reads in symbols and saves them in a trie
    // writes the pairs and outputs a compressed file
    TrieNode *root = trie_create();
    TrieNode *curr_node = root;
    TrieNode *prev_node = NULL;
    uint8_t curr_sym = 0;
    uint8_t prev_sym = 0;
    uint16_t next_code = START_CODE;
    while (read_sym(infile, &curr_sym) == true) {
        TrieNode *next_node = trie_step(curr_node, curr_sym);
        if (next_node != NULL) {
            prev_node = curr_node;
            curr_node = next_node;
        } else {
            write_pair(outfile, curr_node->code, curr_sym, bit_length(next_code));
            curr_node->children[curr_sym] = trie_node_create(next_code);
            curr_node = root;
            next_code = next_code + 1;
        }
        if (next_code == MAX_CODE) {
            trie_reset(root);
            curr_node = root;
            next_code = START_CODE;
        }
        prev_sym = curr_sym;
    }
    if (curr_node != root) {
        write_pair(outfile, prev_node->code, prev_sym, bit_length(next_code));
        next_code = (next_code + 1) % MAX_CODE;
    }
    write_pair(outfile, STOP_CODE, 0, bit_length(next_code));
    flush_pairs(outfile);
    // if the v command line option was specified, print out the stats
    total_syms -= 1;
    if (stats == true) {
        printf("Compressed file size: %lu bytes\n", Bytes(total_bits));
        printf("Uncompressed file size: %lu bytes\n", total_syms);
        float total_bytes = Bytes(total_bits);
        float saving = 100 * (1 - (total_bytes / total_syms));
        printf("Space saving: %4.2lf%%\n", saving);
    }

    trie_delete(root);
    close(infile);
    close(outfile);
    return 0;
}
