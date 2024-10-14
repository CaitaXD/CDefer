#include <stdio.h>
#include <stdlib.h>
#include <Threading/Monitor.h>

#include "defer.h"
#include "Array.h"

int main(void) {

    defer_scope {
        defer {
            printf("End Simple Scope\n");
        }
        printf("Begin Simple Scope\n");
        printf("\tHello World!\n");
    }

    printf("\n==========================================================================================\n\n");

    defer_scope {
        defer {
            printf("End Outer Scope\n");
        }
        printf("Begin Outer Scope\n");
        defer_scope {
            defer {
                printf("\tEnd Inner Scope\n");
            }
            printf("\tBegin Inner Scope\n");\
            printf("\t\tHello World!\n");
        }
    }

    printf("\n==========================================================================================\n\n");

    defer_scope {
        defer {
            defer {
                printf("End Scope Nested Defer\n");
            }
            printf("End Scope\n");
            defer {
                printf("End Scope Nested Defer First\n");
            }
        }
        printf("Begin Scope\n");
    }

    printf("\n==========================================================================================\n\n");

    defer_scope {
        defer {
            printf("End Outer Scope\n");
        }
        printf("Begin Outer Scope\n");
        defer_scope {
            defer {
                defer {
                    printf("\tEnd Inner Scope Nested\n");
                }
                printf("\tEnd Inner Scope\n");
                defer {
                    printf("\tEnd Inner Scope Nested First\n");
                }
            }
            printf("\tBegin Inner Scope\n");\
            printf("\t\tHello World!\n");
        }
    }

    printf("\n==========================================================================================\n\n");

    defer_scope {
        defer {
            printf("End Outer Scope\n");
        }
        printf("Begin Outer Scope\n");
        defer_scope {
            defer {
                defer_scope {
                    printf("\tDefer Scope Inside Defer\n");
                    defer {
                        printf("\tEnd Inner Scope Nested\n");
                    }
                    printf("\tEnd Inner Scope\n");
                    defer {
                        printf("\tEnd Inner Scope Nested First\n");
                    }
                }
            }
            printf("\tBegin Inner Scope\n");\
            printf("\t\tHello World!\n");
        }
    }

    printf("\n==========================================================================================\n\n");

    defer_scope {
        defer {
            printf("End Simple Scope\n");
        }
        printf("Begin Simple Scope\n");
        defer_break {
            printf("Inside Break\n");
        }
        printf("\tHello World! Will not print\n");
    }


    printf("\n==========================================================================================\n\n");


    defer_scope {
        printf("START =====\n");
        defer {
            defer {
                printf("AFTER END =====\n");
            }
            printf("END =====\n");
        }
        defer {
            printf("  Middle\n");
            printf("End\n");
        }
        printf("Begin\n");
        defer_scope {
            defer {
                printf("End Inner Scope\n");
            }
            printf("Bein Inner Scope\n");
        }
        defer_break {
            printf("\nBreak\n");
            printf("Break Line 2\n");
            defer_scope {
                defer {
                    printf("End Of Inner Scope Inside Break\n");
                }
                defer_scope {
                    defer {
                        printf("End Of Super Inner Scope Inside Break\n");
                    }
                    printf("Begin Super Inner Scope Inside Break\n");
                }
                printf("\nInner Scope Inside Break\n");
                printf("Inner Scope Inside Break Line 2\n");
            }
            defer_break {
                printf("Inner Break Cause Why not?\n");
            }
            defer {
                printf("Defer Inner After Break Will not print\n");
            }
            printf("Code after Inner Break Will not print\n");
        }
        defer {
            printf("Defer after Break Will not print\n");
        }
        printf("Code After Break Will not print\n");
    }
    printf("After Scope\n");
    return 0;
}
