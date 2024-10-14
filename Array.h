#ifndef ARRAY_H
#define ARRAY_H

#include "Allocator.h"
#include <stdint.h>
#include <string.h>

#ifndef stack_box
#   define stack_box(X) ((typeof(X)[1]){X})
#endif

#ifndef container_of
#define container_of(ptr__, type__, member__) ((type__*) ((intptr_t)(ptr__) - offsetof(type__, member__)))
#endif

#ifndef ARRAY_API
#define ARRAY_API static inline
#endif

#define EMBED_ARRAY(type__) \
    size_t length;\
    type__ data[]

#define EMBED_SPAN(type__) \
    size_t length;\
    type__ *data

#define DEFINE_ARRAY(tag, type__) \
    struct tag {\
        EMBED_ARRAY(type__);\
    }

#define DEFINE_SPAN(tag, type__) \
    struct tag {\
        EMBED_SPAN(type__);\
    }

struct array_t {
    EMBED_ARRAY(uint8_t);
};

struct span_t {
    EMBED_SPAN(uint8_t);
};

ARRAY_API struct array_t *(array_new)(size_t element_size, size_t length, const struct allocator_t *allocator);

ARRAY_API struct array_t *(array_init)(size_t element_size, size_t length,
                                       uint8_t data[sizeof(struct array_t) + length * element_size]);

ARRAY_API void (array_resize)(struct array_t **array, size_t element_size, size_t length,
                              const struct allocator_t *allocator);

ARRAY_API struct span_t array_span(const struct array_t *array);

ARRAY_API struct span_t (array_slice)(const struct array_t *array, size_t element_size, size_t offset, size_t count);

ARRAY_API struct span_t (array_skip)(const struct array_t *array, size_t element_size, size_t offset);

ARRAY_API struct span_t (array_take)(const struct array_t *array, size_t element_size, size_t count);

ARRAY_API struct span_t (span_slice)(struct span_t span, size_t element_size, size_t offset, size_t count);

ARRAY_API struct span_t (span_shift)(struct span_t span, size_t offset);

#define array_new(type, length__, allocator__) (array_new)(sizeof(type), (length__), (allocator__))
#define array_init(type, length__, data__) (array_init)(sizeof(type), (length__), (data__))
#define stack_array(type, length__)\
    array_init(sizeof(type), (length__), (uint8_t[sizeof(struct array_t) + (length__) * sizeof(type)]){})
#define array(type, ...)\
    container_of(\
        memcpy(\
            stack_array(type, VA_LENGTH(type, __VA_ARGS__))->data,\
            ((type[]) {__VA_ARGS__}),\
            VA_SIZE(type, __VA_ARGS__)\
        ), struct array_t, data\
    )

#define array_resize(array__, length__, allocator__) \
    ({\
        auto __array = (array__);\
        auto __length = (length__);\
        auto __allocator = (allocator__);\
        auto __data = (*__array)->data;\
        (array_resize)((void*)__array, sizeof(*__data), __length, __allocator);\
    })

#define array_slice(array__, offset__, count__) \
    ({\
        auto __array = (array__);\
        auto __offset = (offset__);\
        auto __count = (count__);\
        auto __data = __array->data;\
        size_t __element_size = sizeof(*__data);\
        (array_slice)((void*)__array, __element_size, __offset, __count);\
    })

#define array_skip(array__, offset__)\
    ({\
        auto __array = (array__);\
        auto __offset = (offset__);\
        auto __data = __array->data;\
        size_t __element_size = sizeof(*__data);\
        (array_skip)((void*)__array, __element_size, __offset);\
    })

#define array_take(array__, count__)\
    ({\
        auto __array = (array__);\
        auto __count = (count__);\
        auto __data = __array->data;\
        size_t __element_size = sizeof(*__data);\
        (array_take)((void*)__array, __element_size, __count);\
    })

#define array_span(array__) (array_span)((array__))
#define array_data(type, array__) ((type*) (array__)->data)
#define span_data(type, span__) ((type*) (span__).data)

#define VA_SIZE(type,...) (sizeof((type[]){__VA_ARGS__}))
#define VA_LENGTH(type,...) (sizeof((type[]){__VA_ARGS__})/sizeof(type))

struct array_t *(array_new)(const size_t element_size, const size_t length, const struct allocator_t *allocator) {
    struct array_t *array = alloc(allocator, sizeof(struct array_t) + length * element_size);
    array->length = length;
    return array;
}

struct array_t *(array_init)(size_t element_size, const size_t length,
                             uint8_t data[sizeof(struct array_t) + length * element_size]) {
    struct array_t *array = (void *) data;
    array->length = length;
    return array;
}

void (array_resize)(struct array_t **array, const size_t element_size, const size_t length,
                    const struct allocator_t *allocator) {
    if (*array == NULL) {
        *array = (array_new)(element_size, length, allocator);
    }
    if (length != (*array)->length) {
        dealloc(allocator, *array);
        *array = (array_new)(element_size, length, allocator);
    }
}

struct span_t (array_slice)(const struct array_t *array, const size_t element_size, const size_t offset,
                            const size_t count) {
    return (struct span_t){
        .length = count,
        .data = (uint8_t *) array->data + offset * element_size
    };
}

struct span_t (array_skip)(const struct array_t *array, const size_t element_size, const size_t offset) {
    return (array_slice)(array, element_size, offset, array->length - offset);
}

struct span_t (array_take)(const struct array_t *array, const size_t element_size, const size_t count) {
    return (array_slice)(array, element_size, 0, count);
}

struct span_t (array_span)(const struct array_t *array) {
    return (struct span_t){
        .length = array->length,
        .data = (uint8_t *) array->data
    };
}

struct span_t span_slice(const struct span_t span,
                         const size_t element_size,
                         const size_t offset, const size_t count) {
    return (struct span_t){
        .length = count * element_size,
        .data = span.data + offset * element_size
    };
}

struct span_t span_shift(const struct span_t span, const size_t offset) {
    return (struct span_t){
        .length = span.length - offset,
        .data = span.data + offset
    };
}

#endif //ARRAY_H
