# CDefer
Go style defer (uses statement expression extension be ware)

API

defer_scope:
  Creates a scope from where you can call defer on, implemented using a for loop 
  Usage:
  defer_scope {
    ...
  }

defer(code__):
    Executes the whatever is code__ at the end of the defer_scope, implemented using setjmp

defer_return(...)
    Returns out of a scope while still calling the defered code correctly

using(before__, after__)
    Creates a defer scope and executes before__ at the begining and after__ at the end, the code in before and after share the same scope
    usage: using(FILE * f = fopen("defer_tst.c", "rb"), f && fclose(f))

scoped_expression(before__, ...)
    Executes before__ at the begining of a scope and __VA_ARGS__ at the end, uses a for loop, CAREFUL this is not a defer scope this is a regular C scope 

    
