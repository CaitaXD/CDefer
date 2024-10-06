#ifndef DEFER_H
#define DEFER_H

#include <setjmp.h>
#include <assert.h>
#include <stdbool.h>

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
#define scoped_expression(var__, ...) \
    for (bool LINE_IDENT(__once_guard) = 1; LINE_IDENT(__once_guard); LINE_IDENT(__once_guard) = false) \
    for (var__;LINE_IDENT(__once_guard);LINE_IDENT(__once_guard) = false, ({__VA_ARGS__;}))
#endif

#ifndef DEFER_STACK_SIZE
#   define DEFER_STACK_SIZE 1024
#endif

_Thread_local jmp_buf _TLG_defer_stack[DEFER_STACK_SIZE];
_Thread_local jmp_buf _TLG_return_address;
_Thread_local bool _TLG_should_return = false;
_Thread_local int _TLG_defer_stack_pointer = -1;


static int __defer_stack_unwind(const int sf, const int sp) {
    if (sp > sf) longjmp(_TLG_defer_stack[sp], 1);
    if (_TLG_should_return) longjmp(_TLG_return_address, 1);
    return 1;
}

#define defer_scope \
for (\
    int __defer_stack_frame = _TLG_defer_stack_pointer, __once_guard = 1; __once_guard; __once_guard = 0,\
    __defer_stack_unwind(__defer_stack_frame, _TLG_defer_stack_pointer)\
)\
scoped_expression() /* Safety measure to prevent a wild break from bipassing the finalisers */

#define defer(code__) \
({\
    assert(_TLG_defer_stack_pointer < DEFER_STACK_SIZE && "Maximum defer depth exceeded what are you doing?"); \
    if (setjmp(_TLG_defer_stack[++_TLG_defer_stack_pointer]) != 0) { \
        _TLG_defer_stack_pointer--; \
        code__; \
        continue; \
    }\
})

#define defer_return(...) \
({\
    _TLG_should_return = true; \
    if (setjmp(_TLG_return_address) != 0) { \
        _TLG_should_return = false; \
        return __VA_ARGS__; \
    }\
    continue; \
})

#define using(before__, after__) \
    defer_scope\
        scoped_expression(before__)\
        scoped_expression(defer(after__))

#endif //DEFER_H
