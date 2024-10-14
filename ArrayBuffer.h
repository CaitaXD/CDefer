#ifndef ARRAYBUFFER_H
#define ARRAYBUFFER_H

#include "Array.h"
#include "Allocator.h"

#ifndef ARRAY_API
#    define ARRAY_API static inline
#endif

#define EMBED_ARRAY_BUFFER(type__) \
    struct allocator_t *allocator;\
    size_t capacity;\
    size_t length;\
    type__ data[]

#define DEFINE_ARRAY_BUFFER(tag, type__) \
    struct tag {\
        EMBED_ARRAY_BUFFER(type__);\
    }

struct array_buffer_t {
    struct allocator_t *allocator;
    size_t capacity;
    size_t length;
    uint8_t data[];
};


ARRAY_API struct array_buffer_t *(array_buffer_new)(size_t element_size, size_t capacity,
                                                    struct allocator_t *allocator);

ARRAY_API struct array_buffer_t *(array_buffer_init)(
    size_t element_size, size_t capacity,
    uint8_t data[sizeof(struct array_buffer_t) + element_size * capacity]);

ARRAY_API void (array_buffer_reserve)(struct array_buffer_t **buffer, size_t element_size, size_t capacity);

ARRAY_API void *(array_buffer_grow)(struct array_buffer_t **buffer, size_t element_size);

ARRAY_API void *(array_buffer_shift)(struct array_buffer_t **buffer, size_t element_size, size_t index);

ARRAY_API void *(array_buffer_pop)(struct array_buffer_t **buffer, size_t element_size);

ARRAY_API void (array_buffer_remove_at)(struct array_buffer_t **buffer, size_t index, size_t element_size);

ARRAY_API void (array_buffer_copy)(
    struct array_buffer_t *dst,
    size_t dst_element_size,
    const struct array_buffer_t *src,
    size_t src_element_size
);

#ifndef VOID_EXPR
#   define VOID_EXPR {}
#endif

#define array_buffer_insert(buffer__, index__, element__) \
    ({\
        auto __buffer = (buffer__);\
        auto __data = (*__buffer)->data;\
        size_t __index = (index__);\
        size_t __element_size = sizeof(*__data);\
        *(typeof(*__data)*)(array_buffer_shift)((void*)__buffer, __element_size, __index) = (element__);\
        VOID_EXPR;\
    })

#define array_buffer_push(buffer__, element__) \
    ({\
        auto __buffer = (buffer__);\
        auto __data = (*__buffer)->data;\
        auto __element_size = sizeof(*__data);\
        *(typeof(*__data)*)array_buffer_grow((void*)__buffer, __element_size) = (element__);\
        VOID_EXPR;\
    })

#define array_buffer_pop(buffer__)\
    ({\
        auto __buffer = (buffer__);\
        auto __data = (*__buffer)->data;\
        auto __element_size = sizeof(*__data);\
        (typeof(__data))(array_buffer_pop)((void*)__buffer, __element_size);\
    })

#define array_buffer_remove_at(buffer__, inde__)\
    ({\
        auto __buffer = (buffer__);\
        auto __data = (*__buffer)->data;\
        auto __element_size = sizeof(*__data);\
        (array_buffer_remove_at)((void*)__buffer, (inde__), __element_size);\
    })

#define array_buffer_new(type, capacity__, allocator__)\
    ({\
        auto __allocator = (allocator__);\
        size_t __capacity = (capacity__);\
        size_t __element_size = sizeof(type);\
        (array_buffer_new)(__element_size, __capacity, __allocator);\
    })

#define array_buffer_init(type, buffer__, capacity__) \
    ({\
        auto __buffer = (buffer__);\
        size_t __capacity = (capacity__);\
        size_t __element_size = sizeof(type);\
        (array_buffer_init)(__element_size, __capacity, __buffer);\
    })

#define array_buffer_reserve(buffer__, capacity__) \
    ({\
        auto __buffer = (buffer__);\
        size_t __capacity = (capacity__);\
        size_t __element_size = sizeof(*(*__buffer)->data);\
        (array_buffer_reserve)((void*)__buffer, __element_size, __capacity);\
    })

#define stack_array_buffer(type, capacity__)\
    (array_buffer_init)(sizeof(type), (capacity__), (uint8_t[sizeof(struct array_buffer_t) + (capacity__) * sizeof(type)]){})

#define array_buffer_copy(dst__, src__)\
    ({\
        auto __dst = (dst__);\
        auto __src = (src__);\
        size_t __src_element_size = sizeof(*__src->data);\
        size_t __dst_element_size = sizeof(*__dst->data);\
        (array_buffer_copy)((void*)__dst, __dst_element_size, (void*)__src, __src_element_size);\
    })

struct array_buffer_t *(array_buffer_new)(const size_t element_size, const size_t capacity,
                                          struct allocator_t *allocator) {
    struct array_buffer_t *buffer = alloc(allocator, sizeof(struct array_buffer_t) + element_size * capacity);
    if (buffer == nullptr) return nullptr;
    buffer->capacity = capacity;
    buffer->allocator = allocator;
    buffer->length = 0;
    return buffer;
}


#ifndef min
#   define min(a__, b__) ({\
        auto __a = (a__);\
        auto __b = (b__);\
        __a < __b ? __a : __b;\
    })
#endif

#ifndef max
#   define max(a__, b__) ({\
        auto __a = (a__);\
        auto __b = (b__);\
        __a > __b ? __a : __b;\
    })
#endif

#define null_coalescing(a__, b__) ({\
    auto __a = (a__);\
    auto __b = (b__);\
    __a == nullptr ? __b : __a;\
})

void (array_buffer_reserve)(struct array_buffer_t **buffer, const size_t element_size, const size_t capacity) {
    assert(buffer && *buffer && "Buffer can't be null");
    struct array_buffer_t *buffer_ = *buffer;
    if (buffer_->capacity <= capacity) {
        buffer_->capacity = max(capacity, (size_t)4);
        struct allocator_t *allocator = (*buffer)->allocator;
        struct array_buffer_t *new_buffer = (array_buffer_new)(element_size, buffer_->capacity << 1, allocator);
        assert(new_buffer != nullptr && "Allocation failed, Buy more RAM");
        new_buffer->length = (*buffer)->length;
        memcpy(new_buffer->data, (*buffer)->data, element_size * (*buffer)->length);
        dealloc(buffer_->allocator, *buffer);
        *buffer = new_buffer;
    }
}

void * (array_buffer_grow)(struct array_buffer_t **buffer, const size_t element_size) {
    (array_buffer_reserve)(buffer, element_size, (*buffer)->length + 1);
    void *element = (*buffer)->data + (*buffer)->length * element_size;
    (*buffer)->length += 1;
    return element;
}

void *(array_buffer_pop)(struct array_buffer_t **buffer, const size_t element_size) {
    struct array_buffer_t *const buffer_ = *buffer;
    if (buffer_->length == 0) {
        return NULL;
    }
    void *element = buffer_->data + (buffer_->length - 1) * element_size;
    buffer_->length -= 1;
    return element;
}

void * (array_buffer_shift)(struct array_buffer_t **buffer, const size_t element_size, const size_t index) {
    assert(index <= (*buffer)->length && "Index out of bounds");

    (array_buffer_reserve)(buffer, element_size, (*buffer)->length + 1);
    struct array_buffer_t *buffer_ = *buffer;
    buffer_->length += 1;
    const size_t byte_size = element_size * buffer_->length;
    const size_t src_offset = element_size * index;
    const size_t dst_offset = element_size * (index + 1);
    memmove(
        buffer_->data + dst_offset,
        buffer_->data + src_offset,
        byte_size - src_offset
    );
    void *data = buffer_->data + src_offset;
    return data;
}

void (array_buffer_remove_at)(struct array_buffer_t **buffer, const size_t index, const size_t element_size) {
    assert(index < (*buffer)->length && "Index out of bounds");

    struct array_buffer_t *buffer_ = *buffer;
    const size_t byte_size = element_size * buffer_->length;
    const size_t src_offset = element_size * (index + 1);
    const size_t dst_offset = element_size * index;
    memmove(
        buffer_->data + dst_offset,
        buffer_->data + src_offset,
        byte_size - src_offset
    );
    buffer_->length -= 1;
}

struct array_buffer_t * (array_buffer_init)(
    const size_t element_size,
    const size_t capacity,
    uint8_t data[static sizeof(struct array_buffer_t) + element_size * capacity]) {
    struct array_buffer_t *buffer = (void *) data;
    buffer->capacity = capacity;
    buffer->allocator = nullptr;
    buffer->length = 0;
    const size_t size = element_size * capacity;
    memmove(buffer->data, data, size);
    return buffer;
}

void (array_buffer_copy)(struct array_buffer_t *dst, const size_t dst_element_size,
                         const struct array_buffer_t *src, const size_t src_element_size) {
    if (src_element_size == dst_element_size) {
        memcpy(dst->data, src->data, src_element_size * src->length);
        return;
    }
    for (size_t i = 0; i < src->length; ++i) {
        memcpy(dst->data + i * dst_element_size, src->data + i * src_element_size, dst_element_size);
    }
}
#endif //ARRAYBUFFER_H
