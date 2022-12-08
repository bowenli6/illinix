#include <stdlib.h>
#include <stdio.h>

int main(void) {
    int *p = malloc(sizeof(int));
    *p = 10;
    
    printf("The address of p is %x\n", p); 
    printf("The numebr is %d\n", *p);

    return 0;
}
