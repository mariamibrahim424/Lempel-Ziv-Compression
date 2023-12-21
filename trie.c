#include "trie.h"

#include "code.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// creates a node with a code
// returns pointer to that node
TrieNode *trie_node_create(uint16_t code) {
    TrieNode *n = (TrieNode *) calloc(1, sizeof(TrieNode));
    if (n) {
        n->code = code;
    }
    return n;
}

// delete a node
void trie_node_delete(TrieNode *n) {
    if (n) {
        free(n);
        n = NULL;
    }
    return;
}

// creates a trie which just creates the root with the code EMPTY_CODE
// returns pointer to root
TrieNode *trie_create(void) {
    TrieNode *root = trie_node_create(EMPTY_CODE);
    return root;
}

// deletes all the nodes in the trie except the root
void trie_reset(TrieNode *root) {
    for (uint16_t i = 0; i < ALPHABET; i += 1) {
        if (root->children[i] != NULL) {
            trie_delete(root->children[i]);
            root->children[i] = NULL;
        }
    }
    return;
}
// deletes the trie
void trie_delete(TrieNode *n) {
    if (n) {
        for (uint16_t i = 0; i < ALPHABET; i += 1) {
            trie_delete(n->children[i]);
            n->children[i] = NULL;
        }
        trie_node_delete(n);
    }
    return;
}
// returns a pointer to the child with symbol "sym"
// or NULL if it doesn't exist
TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    return n->children[sym];
}
