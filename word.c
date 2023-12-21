#include "word.h"

#include "code.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// creates a word by copying in the syms inputted
// to the newly allocated space
Word *word_create(uint8_t *syms, uint32_t len) {
    Word *w = (Word *) calloc(1, sizeof(Word));
    if (w) {
        w->len = len;
        w->syms = (uint8_t *) calloc(len, sizeof(uint8_t));
        for (uint8_t i = 0; i < len; i += 1) {
            syms[i] = w->syms[i];
        }
    }
    return w;
}

// append a symbol to a word by creating a new word with the length of the old word+1
// copies in the symbs from the old word into the new word as well as the additional symbol
Word *word_append_sym(Word *w, uint8_t sym) {
    Word *y = (Word *) calloc(w->len + 1, sizeof(Word));
    if (y) {
        y->len = w->len + 1;
        y->syms = (uint8_t *) calloc(w->len, sizeof(uint8_t));
        for (uint8_t i = 0; i < (w->len); i += 1) {
            y->syms[i] = w->syms[i];
        }
        y->syms[w->len] = sym;
    }
    return y;
}

// delete a word
void word_delete(Word *w) {
    if (w) {
        free(w->syms);
        w->syms = NULL;
        free(w);
        w = NULL;
    }
    return;
}

// creates a word table by intilizing the empty word at index 1 or EMPTY_CODE
WordTable *wt_create(void) {
    WordTable *wt = (WordTable *) calloc(MAX_CODE, sizeof(Word *));
    if (wt) {
        wt[EMPTY_CODE] = word_create(NULL, 0);
    }
    return wt;
}

// deletes all the words in the word table expect for the empty word
void wt_reset(WordTable *wt) {
    for (uint32_t i = START_CODE; i < MAX_CODE; i += 1) {
        if (wt[i]) {
            word_delete(wt[i]);
            wt[i] = NULL;
        }
    }
    return;
}
// deletes the word table
void wt_delete(WordTable *wt) {
    for (uint16_t i = EMPTY_CODE; i < MAX_CODE; i += 1) {
        if (wt[i]) {
            word_delete(wt[i]);
            wt[i] = NULL;
        }
    }
    free(wt);
    return;
}
