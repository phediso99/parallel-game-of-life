#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "game-of-life.h"

/* is integer power of 2? */

int is_power_of_2 (unsigned int x)
{
 while (((x & 1) == 0) && x > 1) /* While x is even and > 1 */
   x >>= 1;
 return (x == 1);
}

/* add to a width index, wrapping around like a cylinder */

int xadd (int i, int a, int N) {
  i += a;
  while (i < 0) i += N;
  while (i >= N) i -= N;
  return i;
}

/* add to a height index, wrapping around */

int yadd (int i, int a, int N) {
  i += a;
  while (i < 0) i += N;
  while (i >= N) i -= N;
  return i;
}


int adjacent_to(char *board, int i, int j, int rows, int cols) {
  int k, l;
  int count = 0;

  for (k = -1; k <= 1; k++)
    for (l = -1; l <= 1; l++)
      if (k || l)
        if (board[xadd(i, k, rows) * cols + yadd(j, l, cols)]) count++;

  return count;
}
