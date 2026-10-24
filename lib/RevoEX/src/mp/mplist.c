#include <private/mp.h>
#include <revolution/net.h>

static inline void InsertList(int *p, int *n) {
    MPListNode *position = (MPListNode *)p;
    MPListNode *node = (MPListNode *)n;
    if (!position) {
        return;
    }
    if (!node) {
        return;
    }
    {
        MPListNode *next = position->next;
        node->prev = position;
        node->next = next;
        node->owner = position->owner;
        position->next = node;
        next->prev = node;
    }
}

void MPiInitList(MPListNode *list, MPListNode *nodes, u32 count) {
    u32 i;
    if (list) {
        NETMemSet((void *)(list), (int)(0), (u32)(sizeof(MPListNode)));
        list->prev = list;
        list->next = list;
        list->owner = list;
        if (nodes) {
            for (i = 0; i < count; ++i) {
                InsertList((int *)list->prev, (int *)nodes);
                ++nodes;
            }
        }
    }
}

void MPiInsertList(int *position, int *node) {
    InsertList(position, node);
}

int *MPiRemoveList(int *address) {
    MPListNode *node = (MPListNode *)address;
    MPListNode *prev;
    MPListNode *next;
    if (!node) {
        return NULL;
    }
    if (node == node->owner) {
        return NULL;
    }
    prev = node->prev;
    next = node->next;
    node->prev = NULL;
    node->next = NULL;
    node->owner = NULL;
    prev->next = next;
    next->prev = prev;
    return address;
}
