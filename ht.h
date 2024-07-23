/* ht - simple, single-header hash table implementation.
 *
 * This hash table implementation uses open addressing with linear probing and very simple expansion funciton
 * to handle collisions. It is intended to be used in small projects where a hash table is needed but the speed
 * and efficiency of the hash table is not a priority. Having string as key makes implementation simpler and
 * useablility isn't affected very much - if your data-type can be converted to string, you can use it as key.
 * 
 * By default the hashing function used is fnv1a-1a, but it can be changed by defining a custom hashing function
 * compatible with the signature `uint64_t name(const char*)` and assigning it to hashFunc member of Your HashTable
 * struct, or using one of other provided hashing functions:
 *  - prhf - polynomial rolling hash function
 * 
 * Sample usage:
 * ```c
// #define HT_IMPLEMENTATION
// #include "ht.h"
//
// int main(void) {
//     HashTable* ht = ht_create(10, free);
//     if (ht == NULL) {
//         return 1;
//     }

//     char word[100];
//     while (scanf("%s", word) != EOF) {
//         int* count = (int*) ht_get(ht, word);
//         if (count != NULL) {
//             (*count)++;
//             continue;
//         }

//         count = (int*) malloc (sizeof(int));
//         if (count == NULL) {
//             ht_destroy(ht);
//             return 1;
//         }

//         *count = 1;
//         if (!ht_set(ht, word, count)) {
//             ht_destroy(ht);
//             return 1;
//         }
//     }

//     HashTableIterator* it = ht_iterator(ht);
//     while (ht_next(it)) {
//         printf("%s: %d\n", it->key, *((int*) it->value));
//     }

//     printf("Total words: %zu\n", ht_length(ht));

//     ht_destroy(ht);
//     return 0;
// }
 * ```
 * Copyright (c) Mikołaj Trafisz 2024
 * https://github.com/mtrafisz/ht
 * See LICENSE for more information.
 */

#ifndef _HT_H
#define _HT_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

typedef void (*DestroyFunc)(void*);
typedef uint64_t (*HashFunc)(const char*);

typedef struct {
    char* key;
    void* value;
} HashTableEntry;

typedef struct {
    HashTableEntry* entries;
    uint64_t capacity;
    uint64_t lenght;
    DestroyFunc destroyFunc;
    HashFunc hashFunc;
} HashTable;

typedef struct {
    const char* key;
    void* value;

    HashTable* _ht;
    uint64_t _index;
} HashTableIterator;

HashTable* ht_create(uint64_t size, DestroyFunc destroyFunc);
void ht_destroy(HashTable* ht);

void* ht_get(HashTable* ht, const char* key);
const char* ht_set(HashTable* ht, const char* key, void* value);
#define ht_set_literal(ht, key, value) do {\
    typeof(value)* _value = (typeof(value)*) malloc (sizeof(typeof(value)));\
    *_value = value;\
    ht_set(ht, key, _value);\
} while(0)
size_t ht_length(HashTable* ht);
void* ht_remove(HashTable* ht, const char* key);

HashTableIterator* ht_iterator(HashTable* ht);
bool ht_next(HashTableIterator* it);

#if defined(HT_IMPLEMENTATION) || defined(DEBUG)

uint64_t fnv1a(const char* key) {
    uint64_t hash = 14695981039346656037ULL;
    while (*key) {
        hash ^= (uint64_t) *key++;
        hash *= 1099511628211ULL;
    }

    return hash;
}

uint64_t prhf(const char* key) {
    const uint64_t p = 53, m = 1e9 + 9, n = strlen(key);

    uint64_t hash_value = 0;
    uint64_t p_pow = 1;

    for (uint64_t i = 0; i < n; i++) {
        hash_value = (hash_value + (key[i] - 'a' + 1) * p_pow) % m;
        p_pow = (p_pow * p) % m;
    }
    
    return hash_value;
}

HashTable* ht_create(uint64_t size, DestroyFunc destroyFunc) {
    HashTable* ht = (HashTable*) malloc (sizeof(HashTable));
    if (ht == NULL) {
        return NULL;
    }

    ht->entries = (HashTableEntry*) calloc (size, sizeof(HashTableEntry));
    if (ht->entries == NULL) {
        free(ht);
        return NULL;
    }

    ht->capacity = size;
    ht->lenght = 0;
    ht->destroyFunc = destroyFunc;
    ht->hashFunc = fnv1a;

    return ht;
}

void ht_destroy(HashTable* ht) {
    for (uint64_t i = 0; i < ht->capacity; i++) {
        if (ht->entries[i].key != NULL) {
            free((void*) ht->entries[i].key);
            if (ht->destroyFunc != NULL) {
                ht->destroyFunc(ht->entries[i].value);
            }
        }
    }

    free(ht->entries);
    free(ht);
}

size_t ht_length(HashTable* ht) {
    return ht->lenght;
}

void* ht_get(HashTable* ht, const char* key) {
    uint64_t hash = ht->hashFunc(key);
    uint64_t index = (size_t)(hash & (uint64_t)(ht->capacity - 1));

    while (ht->entries[index].key != NULL) {
        if (strcmp(ht->entries[index].key, key) == 0) {
            return ht->entries[index].value;
        }

        index = (index + 1) % ht->capacity;
    }

    return NULL;
}

int ht_expand(HashTable* ht) {
    uint64_t newCapacity = ht->capacity * 2;
    HashTableEntry* newEntries = (HashTableEntry*) calloc (newCapacity, sizeof(HashTableEntry));
    if (newEntries == NULL) {
        return 0;
    }

    for (uint64_t i = 0; i < ht->capacity; i++) {
        if (ht->entries[i].key != NULL) {
            uint64_t hash = ht->hashFunc(ht->entries[i].key);
            uint64_t index = (size_t)(hash & (uint64_t)(newCapacity - 1));

            while (newEntries[index].key != NULL) {
                index = (index + 1) % newCapacity;
            }

            newEntries[index].key = ht->entries[i].key;
            newEntries[index].value = ht->entries[i].value;
        }
    }

    free(ht->entries);
    ht->entries = newEntries;
    ht->capacity = newCapacity;

    return 1;
}

const char* ht_set(HashTable* ht, const char* key, void* value) {
    if (value == NULL) {
        return NULL;
    }

    if (ht->lenght >= ht->capacity) {
        if (!ht_expand(ht)) {
            return NULL;
        }
    }

    uint64_t hash = ht->hashFunc(key);
    uint64_t index = (size_t)(hash & (uint64_t)(ht->capacity - 1));

    while (ht->entries[index].key != NULL) {
        if (strcmp(ht->entries[index].key, key) == 0) {
            if (ht->destroyFunc != NULL) {
                ht->destroyFunc(ht->entries[index].value);
            }
            ht->entries[index].value = value;
            return key;
        }

        index = (index + 1) % ht->capacity;
    }

    if (ht->entries[index].key == NULL) {
        ht->entries[index].key = strdup(key);
        if (ht->entries[index].key == NULL) return NULL;
        ht->entries[index].value = value;
        ht->lenght++;
    } else {
        return NULL;
    }

    return key;
}

void* ht_remove(HashTable* ht, const char* key) {
    uint64_t hash = ht->hashFunc(key);
    uint64_t index = (size_t)(hash & (uint64_t)(ht->capacity - 1));

    while (ht->entries[index].key != NULL) {
        if (strcmp(ht->entries[index].key, key) == 0) {
            void* value = ht->entries[index].value;
            ht->entries[index].key = NULL;
            ht->entries[index].value = NULL;
            ht->lenght--;
            return value;
        }

        index = (index + 1) % ht->capacity;
    }

    return NULL;
}

HashTableIterator* ht_iterator(HashTable* ht) {
    HashTableIterator* it = (HashTableIterator*) malloc (sizeof(HashTableIterator));
    if (it == NULL) {
        return NULL;
    }

    it->key = NULL;
    it->value = NULL;
    it->_ht = ht;
    it->_index = 0;

    return it;
}

bool ht_next(HashTableIterator* it) {
    while (it->_index < it->_ht->capacity) {
        if (it->_ht->entries[it->_index].key != NULL) {
            it->key = it->_ht->entries[it->_index].key;
            it->value = it->_ht->entries[it->_index].value;
            it->_index++;
            return true;
        }

        it->_index++;
    }

    return false;
}

#endif
#endif
