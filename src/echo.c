#include <stdio.h>


int main(void) {
    const int size = 32;
    char *buffer = malloc(size * sizeof(char));

    if (fgets(buffer, size, stdin))
        fputs(stdout, buffer);

    free(buffer);
    return 0;
}
