#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "game-of-life.h"

/* print the life board */

void print (char *board, int rows, int cols) {
  int i, j;
  int N = cols;

  /* for each row */
  for (i = 0; i < rows; i++) {

    /* print each column position... */
    for (j = 0; j < cols; j++) {
      printf ("%c", board[i * cols + j] ? 'x' : ' ');
    }

    /* followed by a carriage return */
    printf ("\n");
  }
}

/* display the table with delay and clear console */

void display_table(char *board, int rows, int cols) {
  print (board, rows, cols);
  usleep(100000);
  /* clear the screen using VT100 escape codes */
  puts ("\033[H\033[J");
}
