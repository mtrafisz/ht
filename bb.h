#ifndef _BINBUF_H
#define _BINBUF_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct _binbuf_s {
    char* data;
    size_t length;
    size_t capacity;
} BinBuffer;

BinBuffer* bb_create(size_t capacity);
bool bb_destroy(BinBuffer* bb);

bool bb_append(BinBuffer* bb, const char* data, size_t length);
bool bb_append_byte(BinBuffer* bb, char byte);

bool bb_set(BinBuffer* bb, size_t index, char* data, size_t length);
bool bb_set_byte(BinBuffer* bb, size_t index, char byte);

char* bb_get(BinBuffer* bb, size_t index, size_t length);
char bb_get_byte(BinBuffer* bb, size_t index);
char* bb_collect(BinBuffer* bb);    // also frees the BinBuffer

bool bb_expand(BinBuffer* bb, size_t new_capacity);

#ifdef BB_IMPLEMENTATION

BinBuffer* bb_create(size_t capacity) {
    BinBuffer* bb = (BinBuffer*) malloc (sizeof(BinBuffer));
    if (!bb) return NULL;

    bb->data = (char*) malloc (capacity);
    if (!bb->data) {
        free(bb);
        return NULL;
    }

    bb->length = 0;
    bb->capacity = capacity;

    return bb;
}

bool bb_destroy(BinBuffer* bb) {
    if (!bb) return false;
    if (bb->data) free(bb->data);
    free(bb);
    return true;
}

bool bb_append(BinBuffer* bb, const char* data, size_t length) {
    if (!data || length == 0 || !bb) return false;

    if (bb->length + length > bb->capacity) {
        if (!bb_expand(bb, 2 * bb->length)) return false;
    }

    if (memcpy(bb->data + bb->length, data, length) == NULL) return false;
    bb->length += length;

    return true;
}

bool bb_append_byte(BinBuffer* bb, char byte) {
    if (!bb) return false;

    if (bb->length + 1 > bb->capacity) {
        if (!bb_expand(bb, 2 * bb->length)) return false;
    }

    bb->data[bb->length] = byte;
    bb->length++;

    return true;
}

bool bb_set(BinBuffer* bb, size_t index, char* data, size_t length) {
    if (index + length > bb->capacity || !data || length == 0 || !bb) return false;
    if (memcpy(bb->data + index, data, length) == NULL) return false;
    return true;
}

bool bb_set_byte(BinBuffer* bb, size_t index, char byte) {
    if (index + 1 > bb->capacity || !bb) return false;
    bb->data[index] = byte;
    return true;
}

char* bb_get(BinBuffer* bb, size_t index, size_t requested_length) {
    size_t length = requested_length;
    if (index + length > bb->capacity || !bb || length == 0) return NULL;

    char* data = (char*) malloc (length);
    if (!data) return NULL;

    if (memcpy(data, bb->data + index, length) == NULL) {
        free(data);
        return NULL;
    }

    return data;
}

char bb_get_byte(BinBuffer* bb, size_t index) {
    if (index >= bb->capacity || !bb) return 0;

    return bb->data[index];
}

char* bb_collect(BinBuffer* bb) {
    if (!bb) return NULL;

    char* data = (char*) malloc (bb->length);
    if (!data) return NULL;

    if (memcpy(data, bb->data, bb->length) == NULL) {
        free(data);
        return NULL;
    }

    bb_destroy(bb);

    return data;
}

bool bb_expand(BinBuffer* bb, size_t new_capacity) {
    if (new_capacity <= bb->capacity || !bb) return false;

    bb->data = realloc(bb->data, new_capacity);
    if (!bb->data) return false;
    bb->capacity = new_capacity;

    return true;
}

#endif
#endif
