#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <assert.h>
#include <stdint.h>
#include <string.h>

#ifndef ALLOCATOR_API
#define ALLOCATOR_API static inline
#endif

#define ARRAY_LITERAL(size__, ...) (uint8_t[(size__)]){ __VA_ARGS__ }

#ifndef typeof_member
#   define typeof_member(type__, member__) typeof(((type__*)0)->member__)
#endif


// Allocator API

struct allocator_t {
    void * (*alloc)(const struct allocator_t *allocator, size_t size);

    void (*dealloc)(const struct allocator_t *allocator, void *ptr);
};

ALLOCATOR_API void *alloc(const struct allocator_t *allocator, size_t size);

ALLOCATOR_API void dealloc(const struct allocator_t *allocator, void *ptr);

ALLOCATOR_API void *alloc_copy(const struct allocator_t *allocator, const void *ptr, size_t size);

#ifndef DEFAULT_ALLOCATOR

static void *allocator_default_alloc(const struct allocator_t *allocator [[maybe_unused]], const size_t size) {
    return malloc(size);
}

static void allocator_default_dealloc(const struct allocator_t *allocator [[maybe_unused]], void *ptr) {
    free(ptr);
}

static struct allocator_t default_allocator = {
    .alloc = allocator_default_alloc,
    .dealloc = allocator_default_dealloc
};

#else
extern struct allocator_t default_allocator;
#endif
#define DEFAULT_ALLOCATOR &default_allocator

[[maybe_unused]]
static struct allocator_t dummy_allocator = {
    .alloc = nullptr,
    .dealloc = nullptr
};

// Arena Allocator API

struct arena_allocator_t {
    struct allocator_t allocator;
    uintptr_t min_address;
    uintptr_t max_address;
    uintptr_t current_address;
};

ALLOCATOR_API struct arena_allocator_t arena_allocator_init(void *address, size_t bytes);

ALLOCATOR_API void *arena_allocator_alloc(struct arena_allocator_t *allocator, size_t size);

ALLOCATOR_API void arena_allocator_dealloc(const struct arena_allocator_t *allocator, const void *ptr);

ALLOCATOR_API void arena_allocator_clear(struct arena_allocator_t *allocator);

#define stack_arena(size__) arena_allocator_init(ARRAY_LITERAL(size__), size__)

#define static_arena(size__) \
    ({\
        static size_t __arena_size = (size__);\
        static uint8_t __arena_array[__arena_size];\
        arena_allocator_init(__arena_array, __arena_size);\
    })

void *alloc(const struct allocator_t *allocator, const size_t size) {
    if (allocator == nullptr || allocator->alloc == nullptr) {
        return nullptr;
    }
    return allocator->alloc(allocator, size);
}

void dealloc(const struct allocator_t *allocator, void *ptr) {
    if (allocator == nullptr || allocator->dealloc == nullptr) {
        return;
    }
    allocator->dealloc(allocator, ptr);
}

void *alloc_copy(const struct allocator_t *allocator, const void *ptr, const size_t size) {
    void *copy = alloc(allocator, size);
    memcpy(copy, ptr, size);
    return copy;
}

// Arena Allocator API

struct arena_allocator_t arena_allocator_init(void *address, const size_t bytes) {
    return (struct arena_allocator_t){
        .allocator = {
            .alloc = (typeof_member(struct allocator_t, alloc)) arena_allocator_alloc,
            .dealloc = (typeof_member(struct allocator_t, dealloc)) arena_allocator_dealloc
        },
        .current_address = (uintptr_t) address,
        .min_address = (uintptr_t) address,
        .max_address = (uintptr_t) address + bytes,
    };
}

void *arena_allocator_alloc(struct arena_allocator_t *allocator, const size_t size) {
    assert(allocator != NULL && "Allocator is not initialized");
    if (allocator->current_address + size > allocator->max_address) {
        return NULL;
    }
    const uintptr_t ptr = allocator->current_address;
    allocator->current_address += size;
    return (void *) ptr;
}

void arena_allocator_dealloc(const struct arena_allocator_t *allocator, const void *ptr) {
    assert(allocator != NULL && "Arena allocator was not initialized");
    const intptr_t max = allocator->max_address;
    const intptr_t min = allocator->min_address;
    const intptr_t address = (intptr_t) ptr;
    assert(address <= max && address >= min && "Pointer is out of the reserved stack region");
}

void arena_allocator_clear(struct arena_allocator_t *allocator) {
    allocator->current_address = allocator->min_address;
}

#endif //ALLOCATOR_H
