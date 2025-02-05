# CDefer
Go style defer (GCC|Clang only imagine using MSVC) uses setjmp|longjmp and some criminal macros

Example:
```
defer_scope {
  defer {
    printf("End Simple Scope\n");
  } // This Will Execute at the end of the defer scope
  printf("Begin Simple Scope\n");
  printf("\tHello World!\n");
}
```
    
