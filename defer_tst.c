#include <stdio.h>
#include <stdlib.h>
#include "defer.h"

int main(void) {
    using(FILE * f = fopen("defer_tst.c", "rb"), f && fclose(f)) {
        printf("Reading file defer_tst.c\n\n");
        defer(printf("Done!\n"));
        if (f == NULL) {
            printf("File not found\n");
            defer_return(1);
        }
        defer(printf("File Read Successfully\n"));

        char buffer[1024] = {};

        while (1) {
            const size_t read = fread(buffer, sizeof(char), sizeof buffer, f);
            fwrite(buffer, sizeof(char), read, stdout);
            if (read < sizeof buffer) {
                break;
            }
        }
    }

    printf("\n\n==========================================\n\n");

    using(FILE * f = fopen("bob", "rb"), f && fclose(f)) {
        printf("Reading file bob\n\n");
        defer(printf("Done!\n"));
        if (f == NULL) {
            printf("File not found\n");
            defer_return(1);
        }
        defer(printf("File Read Successfully\n"));

        char buffer[1024] = {};

        while (1) {
            const size_t read = fread(buffer, sizeof(char), sizeof buffer, f);
            fwrite(buffer, sizeof(char), read, stdout);
            if (read < sizeof buffer) {
                break;
            }
        }
    }

    return 0;
}
