#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "game-of-life.h"

/* set everything to zero */

void initialize_board (char *board, int rows,  int cols) {
  int   i, j;
  int N = cols;
  
  for (i=0; i<rows; i++)
    for (j=0; j<cols; j++) 
      board[i * cols + j] = 0;
}

/* generate random table */

void generate_table (char *board, int rows, int cols, float threshold) {

  int   i, j;
  int counter = 0;
  int N = cols;

  srand(time(NULL));

  for (j=0; j<cols; j++) {
    for (i=0; i<rows; i++) {
      board[i * cols + j] = ( (float)rand() / (float)RAND_MAX ) < threshold;
      counter += board[i * cols + j];
    }
  }
}

