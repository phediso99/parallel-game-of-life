/* #ifndef UTILS_H_   /\* Include guard *\/ */
/* #define UTILS_H_ */

#define ROOT 0

/* set everything to zero */

void initialize_board (char *board, int rows, int cols);

/* is integer power of 2? */

int is_power_of_2 (unsigned int x);

/* add to a width index, wrapping around like a cylinder */

int xadd (int i, int a, int N);

/* add to a height index, wrapping around */

int yadd (int i, int a, int N);

/* return the number of on cells adjacent to the i,j cell */

int adjacent_to(char *board, int i, int j, int row_size, int col_size);

/* play the game through one generation */

void play_in_serial (char *board, char *newboard, int rows, int cols);
void play_in_parallel(char *board, char *newboard, int rows, int cols);

/* print the life board */

void print (char *board, int rows, int cols);

/* generate random table */

void generate_table (char *board, int rows, int cols, float threshold);

/* display the table with delay and clear console */

void display_table(char *board, int rows, int cols);

/* #endif // FOO_H_ */
