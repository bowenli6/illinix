#include <stdlib.h>
#include <stdio.h>

int main(void) {
    int *p = malloc(1000);
    
    printf("The address of p is %x\n", p); 

    return 0;
}
