#ifndef _DLL_H
#define _DLL_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define each_in_ll(ll, it) ((it) = (ll)->head; (it) != NULL; (it) = (it)->next)

typedef enum {
    LL_HEAD,
    LL_TAIL
} LLInsertionMode;

typedef void (*DestroyFunc)(void*);
typedef bool (*CompareFunc)(void*, void*);

typedef struct _node_s {
    void* value;
    struct _node_s* next;
    struct _node_s* prev;
} Node;

typedef struct {
    Node* head;
    Node* tail;
    uint64_t length;
    DestroyFunc destroyFunc;
} LinkedList;

LinkedList* ll_create(DestroyFunc destroyFunc);
void ll_destroy(LinkedList* ll);

void ll_push(LinkedList* ll, void* value, LLInsertionMode mode);
void* ll_pop(LinkedList* ll, LLInsertionMode mode);
void ll_squeeze_in(LinkedList* ll, void* value, size_t index);  // old one will be pushed to the right
void* ll_remove(LinkedList* ll, size_t index);

void* ll_get(LinkedList* ll, size_t index);
void ll_set(LinkedList* ll, size_t index, void* value);

size_t ll_length(LinkedList* ll);
size_t ll_find(LinkedList* ll, void* value, CompareFunc compareFunc);
void ll_sort(LinkedList* ll, CompareFunc compareFunc);
void ll_reverse(LinkedList* ll);

#ifdef DLL_IMPLEMENTATION

LinkedList* ll_create(DestroyFunc destroyFunc) {
    LinkedList* ll = (LinkedList*) malloc (sizeof(LinkedList));
    ll->head = NULL;
    ll->tail = NULL;
    ll->length = 0;
    ll->destroyFunc = destroyFunc;

    return ll;
}

void ll_destroy(LinkedList* ll) {
    Node* it = ll->head;
    while (it) {
        Node* next = it->next;
        if (ll->destroyFunc) {
            ll->destroyFunc(it->value);
        }
        free(it);
        it = next;
    }

    free(ll);
    ll = NULL;
}

void ll_destroy_nodes(LinkedList* ll) {
    Node* it = ll->head;
    while (it) {
        Node* next = it->next;
        free(it);
        it = next;
    }

    ll->head = NULL;
    ll->tail = NULL;
    ll->length = 0;
}

void ll_push(LinkedList* ll, void* value, LLInsertionMode mode) {
    Node* node = (Node*) malloc (sizeof(Node));
    node->value = value;
    node->next = NULL;
    node->prev = NULL;

    if (ll->head == NULL) {
        ll->head = node;
        ll->tail = node;
    } else {
        if (mode == LL_HEAD) {
            node->next = ll->head;
            ll->head->prev = node;
            ll->head = node;
        } else {
            node->prev = ll->tail;
            ll->tail->next = node;
            ll->tail = node;
        }
    }

    ll->length++;
}

void* ll_pop(LinkedList* ll, LLInsertionMode mode) {
    if (ll->length == 0) {
        return NULL;
    }

    Node* node;
    if (mode == LL_HEAD) {
        node = ll->head;
        ll->head = ll->head->next;
        if (ll->head) {
            ll->head->prev = NULL;
        }
    } else {
        node = ll->tail;
        ll->tail = ll->tail->prev;
        if (ll->tail) {
            ll->tail->next = NULL;
        }
    }

    void* value = node->value;
    free(node);
    node = NULL;
    ll->length--;

    return value;
}

void ll_squeeze_in(LinkedList* ll, void* value, size_t index) {
    if (index > ll->length) {
        return;
    }

    Node* node = (Node*) malloc (sizeof(Node));
    node->value = value;
    node->next = NULL;
    node->prev = NULL;

    if (index == 0) {
        node->next = ll->head;
        ll->head->prev = node;
        ll->head = node;
    } else if (index == ll->length) {
        node->prev = ll->tail;
        ll->tail->next = node;
        ll->tail = node;
    } else {
        Node* it = ll->head;
        for (size_t i = 0; i < index; i++) {
            it = it->next;
        }

        node->next = it;
        node->prev = it->prev;
        it->prev->next = node;
        it->prev = node;
    }

    ll->length++;
}

void* ll_remove(LinkedList* ll, size_t index) {
    if (index >= ll->length) {
        return NULL;
    }

    Node* it = ll->head;
    for (size_t i = 0; i < index; i++) {
        it = it->next;
    }

    if (it->prev) {
        it->prev->next = it->next;
    } else {
        ll->head = it->next;
    }

    if (it->next) {
        it->next->prev = it->prev;
    } else {
        ll->tail = it->prev;
    }

    void* value = it->value;
    free(it);
    it = NULL;
    ll->length--;

    return value;
}

void* ll_get(LinkedList* ll, size_t index) {
    if (index >= ll->length) {
        return NULL;
    }

    Node* it = ll->head;
    for (size_t i = 0; i < index; i++) {
        it = it->next;
    }

    return it->value;
}

void ll_set(LinkedList* ll, size_t index, void* value) {
    if (index >= ll->length) {
        return;
    }

    Node* it = ll->head;
    for (size_t i = 0; i < index; i++) {
        it = it->next;
    }

    it->value = value;
}

size_t ll_length(LinkedList* ll) {
    return ll->length;
}

size_t ll_find(LinkedList* ll, void* value, CompareFunc compareFunc) {
    Node* it = ll->head;
    for (size_t i = 0; i < ll->length; i++) {
        if (compareFunc(it->value, value)) {
            return i;
        }
        it = it->next;
    }

    return ll->length;
}

void ll_sort(LinkedList* ll, CompareFunc compareFunc) {
    if (ll->length <= 1) {
        return;
    }

    LinkedList* left = ll_create(ll->destroyFunc);
    LinkedList* right = ll_create(ll->destroyFunc);

    Node* it = ll->head;
    for (size_t i = 0; i < ll->length / 2; i++) {
        ll_push(left, it->value, LL_TAIL);
        it = it->next;
    }

    for (size_t i = ll->length / 2; i < ll->length; i++) {
        ll_push(right, it->value, LL_TAIL);
        it = it->next;
    }

    ll_sort(left, compareFunc);
    ll_sort(right, compareFunc);

    Node* leftIt = left->head;
    Node* rightIt = right->head;
    Node* llIt = ll->head;

    while (leftIt && rightIt) {
        if (compareFunc(leftIt->value, rightIt->value)) {
            llIt->value = leftIt->value;
            leftIt = leftIt->next;
        } else {
            llIt->value = rightIt->value;
            rightIt = rightIt->next;
        }
        llIt = llIt->next;
    }

    while (leftIt) {
        llIt->value = leftIt->value;
        leftIt = leftIt->next;
        llIt = llIt->next;
    }

    while (rightIt) {
        llIt->value = rightIt->value;
        rightIt = rightIt->next;
        llIt = llIt->next;
    }

    ll_destroy_nodes(left);
    ll_destroy_nodes(right);
    free(left);
    free(right);
}

void ll_reverse(LinkedList* ll) {
    Node* it = ll->head;
    while (it) {
        Node* next = it->next;
        it->next = it->prev;
        it->prev = next;
        it = next;
    }

    Node* temp = ll->head;
    ll->head = ll->tail;
    ll->tail = temp;
}

#endif
#endif
