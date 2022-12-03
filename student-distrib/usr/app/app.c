#include <stdio.h>
#include "string.h"
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    unsigned int fact = 1024;
    unsigned int weight = 39045157;
    printf("%lu\n", (unsigned long) fact * weight);
    return 0;
}   
