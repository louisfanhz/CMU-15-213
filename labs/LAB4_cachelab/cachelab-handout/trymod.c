#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) 
{
    typedef struct {
        int a;
        int b;
    }sb_t;

    sb_t *sb = (sb_t *)malloc(sizeof(sb_t));
    sb->a = 3;
    sb->b = 0;

    printf("%d\n", ++sb->a);

    free(sb);
    return 1;
}
