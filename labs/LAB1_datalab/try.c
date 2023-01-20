#include <stdio.h>

int main(int argc, char *argv[])
{
  unsigned a = 0x7f000000 >> 23;
  printf("%d\n", a);
  return -1;
}
