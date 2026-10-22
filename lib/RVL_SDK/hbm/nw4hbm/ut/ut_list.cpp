#include "revolution/hbm/nw4hbm/ut/list.h"

#include "revolution/hbm/HBMAssert.hpp"

#define OBJ_TO_NODE(list_, object_) reinterpret_cast<Link*>(reinterpret_cast<u32>(object_) + (list_)->offset)

namespace nw4hbm {
namespace ut {

void List_Init(List* list, u16 offset) {
    NW4HBMAssertPointerNonnull_Line(list, 41);
    list->headObject = NULL;
    list->tailObject = NULL;
    list->numObjects = 0;
    list->offset = offset;
}

static void SetFirstObject(List* list, void* object) {
    NW4HBMAssertPointerNonnull_Line(list, 64);
    NW4HBMAssertPointerNonnull_Line(object, 65);

    Link* link = OBJ_TO_NODE(list, object);

    link->nextObject = NULL;
    link->prevObject = NULL;
    list->headObject = object;
    list->tailObject = object;
    list->numObjects++;
}

void List_Append(List* list, void* object) {
    NW4HBMAssertPointerNonnull_Line(list, 89);
    NW4HBMAssertPointerNonnull_Line(object, 90);

    if (list->headObject == NULL) {
        SetFirstObject(list, object);
        return;
    }

    Link* link = OBJ_TO_NODE(list, object);

    link->prevObject = list->tailObject;
    link->nextObject = NULL;

    OBJ_TO_NODE(list, list->tailObject)->nextObject = object;

    list->tailObject = object;
    list->numObjects++;
}

void List_Remove(List* list, void* object) {
    NW4HBMAssertPointerNonnull_Line(list, 203);
    NW4HBMAssertPointerNonnull_Line(object, 204);

    Link* link = OBJ_TO_NODE(list, object);

    if (!link->prevObject) {
        list->headObject = link->nextObject;
    } else {
        OBJ_TO_NODE(list, link->prevObject)->nextObject = link->nextObject;
    }

    if (!link->nextObject) {
        list->tailObject = link->prevObject;
    } else {
        OBJ_TO_NODE(list, link->nextObject)->prevObject = link->prevObject;
    }

    link->prevObject = NULL;
    link->nextObject = NULL;

    list->numObjects--;
}

void* List_GetNext(const List* list, const void* object) {
    NW4HBMAssertPointerNonnull_Line(list, 245);

    if (object == NULL) {
        return list->headObject;
    }

    return OBJ_TO_NODE(list, object)->nextObject;
}

void* List_GetNth(const List* list, u16 index) {
    int count = 0;
    Link* object = NULL;

    NW4HBMAssertPointerNonnull_Line(list, 297);

    for (; (object = static_cast<Link*>(List_GetNext(list, object))); count++) {
        if (index == count) {
            return object;
        }
    }

    return NULL;
}

} // namespace ut
} // namespace nw4hbm
