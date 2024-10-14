#ifndef DEFER_H
#define DEFER_H

#include "ArrayBuffer.h"
#include "Allocator.h"
#include <setjmp.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include "VirtualAllocator.h"

#include <stdatomic.h>
#if defined(__unix__) || defined(__linux__)
#   include <unistd.h>
#endif

#ifndef CONCAT_
#   define CONCAT_(a,b) a##b
#endif

#ifndef CONCAT
#   define CONCAT(a,b) CONCAT_(a,b)
#endif

#ifndef LINE_IDENT
#   define LINE_IDENT(name) CONCAT(name, __LINE__)
#endif


#ifndef scoped_expression
#   define scoped_expression(var__, ...) \
        for (volatile bool LINE_IDENT(__once_guard) = true; LINE_IDENT(__once_guard); LINE_IDENT(__once_guard) = false) \
        for (var__; LINE_IDENT(__once_guard); LINE_IDENT(__once_guard) = false, ({__VA_ARGS__;}))
#endif

#ifndef finalizer_expression
#   define finalizer_expression(expression__) scoped_expression(, expression__)
#endif

#define aligned_to(size__, alignment__) (((size__) + (alignment__)-1) & ~((alignment__)-1))

constexpr size_t GB = 1 << 30;
constexpr size_t MB = 1 << 20;
constexpr size_t KB = 1 << 10;

_Thread_local static jmp_buf *_tls_defer_stack = nullptr;
_Thread_local static int _tls_defer_stack_pointer = -1;
_Thread_local static size_t _tls_defer_cap = 0;

_Thread_local static jmp_buf _tls_defer_anchor_buf;
_Thread_local static bool _tls_defer_jump_anchor = false;

#if defined(_WIN32)
#   include <windows.h>
#elif defined(__unix__) || defined(__linux__)
#   include <unistd.h>
#endif

static pthread_mutex_t __initialization_mutex = PTHREAD_MUTEX_INITIALIZER;
_Thread_local static bool defer_runtime_initialized = false;

static void _defer_runtime_init() {
    if (defer_runtime_initialized) return;
    pthread_mutex_lock(&__initialization_mutex);
    if (_tls_defer_stack == nullptr) {
        _tls_defer_stack = virtual_alloc(
            nullptr,
            GB,
            VMEM_RESERVE,
            VMEM_READ_WRITE
        );
    }
    defer_runtime_initialized = true;
    pthread_mutex_unlock(&__initialization_mutex);
}

static void _defer_reserve_stack(const size_t size [[maybe_unused]]) {
    #if defined(_WIN32)
    const size_t to_commit = virtual_page_size();
    if (size >= _tls_defer_cap) {
        void *adress = _tls_defer_stack + _tls_defer_cap;
        virtual_alloc(adress, to_commit, VMEM_COMMIT, VMEM_READ_WRITE);
        _tls_defer_cap += to_commit;
    }
    #endif
}

static int _defer_stack_unwind(jmp_buf *stack, const int sf, const int sp, const int code) {
    if (sp >= sf) { longjmp(stack[sp], 1); }
    if (_tls_defer_jump_anchor) {
        _tls_defer_jump_anchor = false;
        longjmp(_tls_defer_anchor_buf, code);
    }
    return sp;
}

#define defer_scope \
    scoped_expression(_defer_runtime_init())\
    scoped_expression(_defer_reserve_stack(_tls_defer_stack_pointer + 1))\
    scoped_expression(\
        int __sf = ++_tls_defer_stack_pointer,\
        _defer_stack_unwind(_tls_defer_stack, __sf + 1, _tls_defer_stack_pointer, 1)\
    )\
    if (setjmp(_tls_defer_stack[_tls_defer_stack_pointer]) == 1)\
        _tls_defer_stack_pointer -= 1;\
    else
#define defer \
    scoped_expression(_defer_reserve_stack(_tls_defer_stack_pointer + 1))\
    if (setjmp(_tls_defer_stack[++_tls_defer_stack_pointer]) == 1)\
        finalizer_expression(_defer_stack_unwind(_tls_defer_stack, __sf, _tls_defer_stack_pointer, 1))\
        scoped_expression(_tls_defer_stack_pointer -= 1)

#define defer_break \
    if (setjmp(_tls_defer_anchor_buf) == 0)\
    {\
        _tls_defer_jump_anchor = true;\
        _defer_stack_unwind(_tls_defer_stack, __sf, _tls_defer_stack_pointer, 1);\
    }\
    else\
        finalizer_expression(longjmp(_tls_defer_stack[__sf], 1))\
        scoped_expression(_tls_defer_stack_pointer++)

#define using(before__, after__) \
    defer_scope\
        scoped_expression(before__)\
        scoped_expression( ({defer { after__; };}) )

#define using_break defer_break

#endif //DEFER_H
