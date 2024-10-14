#ifndef VIRTUALALLOCATOR_H
#define VIRTUALALLOCATOR_H

#include <stddef.h>

#ifndef VMEM_API
#   define VMEM_API static inline
#endif

enum vmem_protection;
enum vmem_alloc_type; // Flags

VMEM_API void *virtual_alloc(void *ptr, size_t size, enum vmem_alloc_type alloc_type, enum vmem_protection protection);

VMEM_API int virtual_protect(void *ptr, size_t size, enum vmem_protection protection);

VMEM_API int virtaul_dealloc(void *ptr, size_t size, enum vmem_alloc_type alloc_type);

VMEM_API size_t virtual_page_size();

VMEM_API size_t virtual_granularity();

#if defined(__unix__) || defined(__linux__)
#   include <sys/mman.h>
#   include <unistd.h>

enum vmem_protection {
    VMEM_NO_ACCESS,
    VMEM_READ_WRITE,
};

enum vmem_alloc_type {
    VMEM_NOOP = 0,
    VMEM_COMMIT = VMEM_NOOP, // On Linux memory is automatically committed
    VMEM_RESERVE = 1 << 0,
    VMEM_DECOMMIT = 1 << 1,
    VMEM_RELEASE = 1 << 2,
};

static int vmem_protection_posix(const enum vmem_protection protection) {
    switch (protection) {
        case VMEM_NO_ACCESS:
            return PROT_NONE;
        case VMEM_READ_WRITE:
            return PROT_READ | PROT_WRITE;
        default:
            return -1;
    }
}

void *virtual_alloc(
    void *ptr,
    const size_t size,
    const enum vmem_alloc_type alloc_type,
    const enum vmem_protection protection) {
    if (alloc_type == VMEM_NOOP) { return ptr; }
    return mmap(ptr, size, vmem_protection_posix(protection), MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

int virtual_protect(void *ptr, const size_t size, const enum vmem_protection protection) {
    return mprotect(ptr, size, vmem_protection_posix(protection));
}

int virtaul_dealloc(void *ptr, const size_t size, const enum vmem_alloc_type alloc_type) {
    if (alloc_type & VMEM_RELEASE) {
        return munmap(ptr, size);
    }
    if (alloc_type & VMEM_DECOMMIT) {
#if __USE_MISC
        return madvise(ptr, size, MADV_DONTNEED);
#else
        return 0;
#endif
    }
    return -1;
}

size_t virtual_page_size() {
    return sysconf(_SC_PAGESIZE);
}

size_t virtual_granularity() {
    return sysconf(_SC_PAGE_SIZE);
}

#elif defined(_WIN32)
#   include <windows.h>
#   include <memoryapi.h>

enum vmem_protection {
    VMEM_NO_ACCESS = PAGE_NOACCESS,
    VMEM_READ_WRITE = PAGE_READWRITE,
};

enum vmem_alloc_type {
    VMEM_NOOP = 0,
    VMEM_COMMIT = MEM_COMMIT,
    VMEM_RESERVE = MEM_RESERVE,
    VMEM_DECOMMIT = MEM_DECOMMIT,
    VMEM_RELEASE = MEM_RELEASE,
};

static int vmem_protection_windows(const enum vmem_protection protection) {
    return protection;
}

static int vmem_alloc_type_windows(const enum vmem_alloc_type alloc_type) {
    return alloc_type;
}

void *virtual_alloc(void *ptr, const size_t size, const enum vmem_alloc_type alloc_type,
                    const enum vmem_protection protection) {
    if (alloc_type == VMEM_NOOP) { return ptr; }
    return VirtualAlloc(ptr, size, vmem_alloc_type_windows(alloc_type), vmem_protection_windows(protection));
}

int virtual_protect(void *ptr, const size_t size, const enum vmem_protection protection) {
    return VirtualProtect(ptr, size, vmem_protection_windows(protection), nullptr);
}

int virtaul_dealloc(void *ptr, const size_t size, const enum vmem_alloc_type alloc_type) {
    return VirtualFree(ptr, size, vmem_alloc_type_windows(alloc_type));
}

size_t virtual_page_size() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwPageSize;
}

size_t virtual_granularity() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwAllocationGranularity;
}

#endif

struct virtual_arena_t {
    struct allocator_t allocator;
    uintptr_t min_address;
    uintptr_t max_address;
    uintptr_t current_address;
    #if defined(_WIN32)
    uintptr_t commited;
    #endif
};

VMEM_API struct virtual_arena_t virtual_arena_init(size_t size);

VMEM_API void *virtual_arena_alloc(struct virtual_arena_t *allocator, size_t size);

VMEM_API void virtual_arena_dealloc(const struct virtual_arena_t *allocator, void *ptr);

VMEM_API void virtual_arena_clear(struct virtual_arena_t *allocator);

struct virtual_arena_t virtual_arena_init(const size_t size) {
    constexpr enum vmem_protection rw = VMEM_READ_WRITE;
    constexpr enum vmem_alloc_type reserve = VMEM_RESERVE;
    const uintptr_t address = (uintptr_t) virtual_alloc(nullptr, size, reserve, rw);
    return (struct virtual_arena_t){
        .allocator = {
            .alloc = (typeof_member(struct allocator_t, alloc)) virtual_arena_alloc,
            .dealloc = (typeof_member(struct allocator_t, dealloc)) virtual_arena_dealloc
        },
        .min_address = (uintptr_t) address,
        .max_address = (uintptr_t) address + size,
        .current_address = (uintptr_t) address,
    };
}

void *virtual_arena_alloc(struct virtual_arena_t *allocator, const size_t size) {
    if (allocator->current_address + size > allocator->max_address) {
        return nullptr;
    }
    constexpr enum vmem_protection rw = VMEM_READ_WRITE;
    constexpr enum vmem_alloc_type commit = VMEM_COMMIT;
    auto const ptr = (void *) allocator->current_address;
    allocator->current_address += size;
    #if defined(_WIN32)
    if (allocator->current_address >= allocator->commited) {
        const size_t page_size = virtual_page_size();
        virtual_alloc(ptr, page_size, commit, rw);
        allocator->commited += page_size;
    }
    #endif
    return ptr;
}

void virtual_arena_dealloc(const struct virtual_arena_t *allocator, void *ptr) {
    const uintptr_t address = (uintptr_t) ptr;
    assert(address >= allocator->min_address &&
        address <= allocator->max_address &&
        "Pointer is out of the reserved region");
}

void virtual_arena_clear(struct virtual_arena_t *allocator) {
    const size_t size = allocator->max_address - allocator->min_address;
    virtaul_dealloc((void *) allocator->min_address, size, VMEM_DECOMMIT);
    allocator->current_address = allocator->min_address;
}

#endif //VIRTUALALLOCATOR_H
