#include <stdio.h>
#include <stdlib.h>

#define PUT_ADR(p, val)     (*(unsigned int *)(p) = (unsigned int)(val))
#define MIN_SIZE 16
#define WSIZE 4

static int get_listno(size_t size);

int main(int argc, char** argv) {
    void *roots_listp;
    roots_listp = malloc(12*sizeof(unsigned int));
    for (int i=0; i < 12; i++) {
        *((unsigned int*)roots_listp+i) = (unsigned int)NULL;
    }

    /*for (int i=0; i < 12; i++) {
        printf("0x%x, ", *((unsigned int *)roots_listp+i));
    }
    printf("\n");
    */

    printf("%d\n", get_listno(atoi(argv[1])));
    
    return 1;
}

static int get_listno(size_t size) {
    size_t words = (size-MIN_SIZE)/WSIZE;
    if (words <= 2)
        return 0;
    if (words <= 4)
        return 1;
    if (words <= 8)
        return 2;
    if (words <= 16)
        return 3;
    if (words <= 32)
        return 4;
    if (words <= 64)
        return 5;
    if (words <= 128)
        return 6;
    if (words <= 256)
        return 7;
    if (words <= 512)
        return 8;
    return 9;
}

