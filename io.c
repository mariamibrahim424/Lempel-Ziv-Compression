#include "io.h"

#include "code.h"
#include "endian.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static uint8_t pairs_buf[BLOCK];
static uint8_t sym_buf[BLOCK];
static uint16_t bit_index = 0;
static int sym_index = 0;
static int end_of_buf = -1;
uint64_t total_syms = 0; // To count the symbols processed.
uint64_t total_bits = 0; // To count the bits processed.

// takes in number of bits and returns the bytes necessary to represnt these bits
uint32_t bytes(uint32_t bits) {
    if (bits % 8 == 0) {
        return (bits / 8);
    } else {
        return (bits / 8 + 1);
    }
}

// imitates the function read()
// reads in bytes returns number of bytes that were read
int read_bytes(int infile, uint8_t *buf, int to_read) {
    int total = 0;
    int bytes_read = 0;
    int bytes_to_read = to_read;
    while ((to_read != total) && ((bytes_read = read(infile, buf + total, bytes_to_read)) > 0)) {
        total += bytes_read;
        bytes_to_read -= bytes_read;
    }
    return total;
}
// imitates the function write()
// write bytes and returns number of bytes written out
int write_bytes(int outfile, uint8_t *buf, int to_write) {
    int total = 0;
    int bytes_written = 0;
    int bytes_to_write = to_write;
    while ((to_write != total)
           && ((bytes_written = write(outfile, buf + total, bytes_to_write)) > 0)) {
        total += bytes_written;
        bytes_to_write -= bytes_written;
    }
    return total;
}

// used for decoding
// reads the bytes of a header in a file
void read_header(int infile, FileHeader *header) {
    total_bits += 8 * sizeof(FileHeader);
    read_bytes(infile, (uint8_t *) header, sizeof(FileHeader));
    return;
}

// used for encoding
// writes the bytes of a header in a file
void write_header(int outfile, FileHeader *header) {
    total_bits += 8 * sizeof(FileHeader);
    write_bytes(outfile, (uint8_t *) header, sizeof(FileHeader));
    return;
}

// inspired by Eugene's code from section
// used in encoding
// reads in a symbol and returns false if this symbol hit the end of buffer
// otherwise true
bool read_sym(int infile, uint8_t *sym) {
    // if the buffer is empty, fill it up
    total_syms += 1;
    if (sym_index == 0) {
        int bytes_read = read_bytes(infile, sym_buf, BLOCK);
        // update the end of buffer with the bytes read in
        if (bytes_read < BLOCK) {
            end_of_buf = bytes_read + 1;
        }
    }
    *sym = sym_buf[sym_index];
    sym_index += 1;
    // if end of buffer is reached, reset it
    if (sym_index == BLOCK) {
        sym_index = 0;
    }
    return sym_index == end_of_buf ? false : true;
}

// inspired by Eugene's code from section
// used in encoding
// buffers a code and its symbol to pairs_buf
void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) {
    total_bits += bitlen + 8;
    // loops through the bitlen code
    // sets the necessary bits of the code into the buffer
    for (int i = 0; i < bitlen; i += 1) {
        uint8_t bit = (code >> i) & 1;
        if (bit == 1) {
            pairs_buf[bit_index / 8] |= (1 << (bit_index % 8));
        } else {
            pairs_buf[bit_index / 8] &= ~(1 << (bit_index % 8));
        }
        bit_index += 1;
        // if a BLOCK is read then the buffer has to be written out and reset
        if (bit_index == (BLOCK * 8)) {
            write_bytes(outfile, pairs_buf, BLOCK);
            bit_index = 0;
        }
    }
    // loops through the 8 bit symbol
    // sets the necessary bits of the symbol into the buffer
    for (int j = 0; j < 8; j += 1) {
        uint8_t bit = (sym >> j) & 1;
        if (bit == 1) {
            pairs_buf[bit_index / 8] |= (1 << (bit_index % 8));
        } else {
            pairs_buf[bit_index / 8] &= ~(1 << (bit_index % 8));
        }
        bit_index += 1;
        // if a BLOCK is read then the buffer has to be written out and reset
        if (bit_index == (BLOCK * 8)) {
            write_bytes(outfile, pairs_buf, BLOCK);
            bit_index = 0;
        }
    }
    return;
}

// used in encoding
// clears out/resets the pairs_buf
void flush_pairs(int outfile) {
    if (bit_index > 0) {
        write_bytes(outfile, pairs_buf, bytes(bit_index));
    }
    bit_index = 0;
    return;
}

// used in decoding
// inspired by Eugene's code from section
bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) {
    total_bits += bitlen + 8;
    uint16_t temp_code = 0;
    for (int i = 0; i < bitlen; i += 1) {
        if (bit_index == 0) {
            read_bytes(infile, pairs_buf, BLOCK);
        }
        // gets bit of code at bit_index location
        uint8_t bit = (pairs_buf[bit_index / 8] & (1 << bit_index % 8)) >> (bit_index % 8);
        // if that bit is set
        if (bit == 1) {
            // set its corresponding bit in temp_code
            temp_code |= (1 << i);
        } else {
            // clear corresponding bit in temo_code
            temp_code &= ~(1 << i);
        }
        bit_index += 1;
        if (bit_index == (BLOCK * 8)) {
            bit_index = 0;
        }
    }
    // if there are no more pairs to read return false
    *code = temp_code;
    if (*code == STOP_CODE) {
        return false;
    }
    uint8_t temp_sym = 0;
    for (int j = 0; j < 8; j += 1) {
        read_bytes(infile, pairs_buf, BLOCK);
        // gets bit of code at bit_index location
        uint8_t bit = (pairs_buf[bit_index / 8] & (1 << bit_index % 8)) >> (bit_index % 8);
        // if that bit is set
        if (bit == 1) {
            // set its corresponding bit in temp_code
            temp_sym |= (1 << j);
        } else {
            // clear corresponding bit in temo_code
            temp_sym &= ~(1 << j);
        }
        bit_index += 1;
        if (bit_index == (BLOCK * 8)) {
            bit_index = 0;
        }
        *sym = temp_sym;
    }
    return true;
}

// used in decoding
// copies the symbols to form a word
void write_word(int outfile, Word *w) {
    total_syms += 1;
    for (uint32_t i = 0; i < w->len; i += 1) {
        sym_buf[sym_index] = w->syms[i];
        sym_index += 1;
    }
    if (sym_index == BLOCK) {
        write_bytes(outfile, sym_buf, BLOCK);
        sym_index = 0;
    }
    return;
}

// used in decoding
// clears out/resets the sym_buf
void flush_words(int outfile) {
    if (sym_index > 0) {
        write_bytes(outfile, sym_buf, sym_index);
    }
    sym_index = 0;
    return;
}
