#include <mpi.h>
#include <omp.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "game-of-life.h"

int main (int argc, char *argv[]) {

	/*----------------------------------------------------------------------*/
	/*                            STEP 1: Setup                             */
	/*----------------------------------------------------------------------*/

	if (argc != 5) { // Check if the command line arguments are correct
		printf("Usage: %s N thres t disp\n"
		       "where\n"
		       "  N       : size of board chunk in every process (N x N)\n"
		       "  thres   : propability of alive cell\n"
		       "  t       : number of generations\n"
		       "  disp    : {1: display output, 0: hide output}\n"
		       , argv[0]);
		return (1);
	}

	int N = atoi(argv[1]);        /* Array size in every process */
	double thres = atof(argv[2]); /* Propability of life cell */
	int t = atoi(argv[3]);        /* Number of generations */
	int disp = atoi(argv[4]);     /* Display output? */

	char 	    *board,				/* the game board */
	            *validboard,		/* the validation board */
	            *newboard;			/* a new board needed for the validation */


	int	rc, 				/* return error code */
	    processes,			/* total number of MPI processes */
	    pid, 			    /* process identifier */
	    prevproc,			/* previous process */
	    nextproc,			/* next process */
	    arraymsg = 1,		/* setting a message type */
	    i,					/* iteration variable */
	    k, 					/* iteration variable */
	    j,					/* iteration variable */
	    cold,				/* to find the number of totalcols */
	    rowd,				/* to find the number of totalrows */
	    totalrows,			/* number of rows for the total board */
	    totalcols;			/* number of columns for the total board */

	double serialstarttime,	/* to measure starting time */
	       parallelstarttime;

	board = NULL;
	validboard = NULL;
	newboard = NULL;

	/*------------------------------*/
	/* Set number of OpenMP threads */
	/*------------------------------*/
	omp_set_num_threads(8);

	/*-------------------------------------------------------*/
	/* Initialize MPI and obtain  number of processes and ID */
	/*-------------------------------------------------------*/
	rc = MPI_Init(&argc, &argv);
	rc |= MPI_Comm_size(MPI_COMM_WORLD, &processes);
	rc |= MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	if (rc != 0)
		printf ("ERROR: Initializing MPI and obtaining task ID information\n");

	/*------------------------------------------*/
	/* Is the number of processes a power of 2? */
	/*------------------------------------------*/
	if (!is_power_of_2(processes))
		return (1);
	/*------------------------------------*/
	/* Broadcast N and t to all processes */
	/*------------------------------------*/
	MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&t, 1, MPI_INT, 0, MPI_COMM_WORLD);
	/*--------------------------------*/
	/* Find next and previous process */
	/* 0 ->1 ->2 ->3 ->4 -> 0         */
	/*--------------------------------*/
	if (pid == 0)
		prevproc = processes - 1;
	else
		prevproc = pid - 1;
	if (pid == processes - 1)
		nextproc = 0;
	else
		nextproc = pid + 1;

	/*----------------------------------------*/
	/* Find the dimensions of the total board */
	/* depending on N and number of processes */
	/*----------------------------------------*/

	cold = (int)log2(processes) / 2;
	rowd = (int)log2(processes) - cold;

	totalrows = pow(2, rowd) * N;
	totalcols = pow(2, cold) * N;

	int subrows = totalrows / processes;
	int subcols = totalcols;

	/*----------------------------------------------*/
	/* ROOT: Initialize, generate and display board */
	/*----------------------------------------------*/
	if (pid == ROOT) {

		printf("Chunk size per process = %d x %d,  with propability: %0.1f%%\n", N, N, thres * 100);
		printf("Display = %d, generations = %d and processes = %d \n", \
		       disp, t, processes);

		board = (char *)malloc(sizeof(char) * totalrows * totalcols);
		assert(board != NULL);
		initialize_board (board, totalrows, totalcols);
		generate_table (board, totalrows, totalcols, thres);
		validboard = (char *)malloc(sizeof(char) * totalrows * totalcols);
		assert(validboard != NULL);
		memcpy(validboard, board, sizeof(char) * totalrows * totalcols);
		newboard = (char *)malloc(sizeof(char) * totalrows * totalcols);
		assert(newboard != NULL);

		if (disp)
			display_table (board, totalrows, totalcols);
	}

	/*----------------------------------------------------------------*/
	/* Create a buffer that will hold a sub-board of the entire array */
	/* and the other arguments needed for the play function           */
	/*----------------------------------------------------------------*/
	int subboard_size = subrows * subcols;
	char *subboard = (char *)malloc(sizeof(char) * subboard_size);
	assert(subboard != NULL);
	char *newsubboard = (char *)malloc(sizeof(char) * subboard_size);
	assert(newsubboard != NULL);

	int neighbor_size = subcols;
	char *topborderboard = (char *)malloc(sizeof(char) * (3 * subcols));
	assert(topborderboard != NULL);

	char *bottomborderboard = (char *)malloc(sizeof(char) * (3 * subcols));
	assert(bottomborderboard != NULL);

	char *newborderboard = (char *)malloc(sizeof(char) * (3 * subcols));
	assert(newborderboard != NULL);

	/*----------------------------------------------------------------------*/
	/*                       STEP 2: Divide and play                        */
	/*----------------------------------------------------------------------*/
	if (pid == ROOT)
		parallelstarttime = MPI_Wtime();

	/*-------------------------------------------------------*/
	/* Scatter the array chunks of elements in all processes */
	/* in the MPI world                                      */
	/*-------------------------------------------------------*/
	MPI_Scatter(board, subboard_size, MPI_CHAR,
	            subboard, subboard_size, MPI_CHAR,
	            ROOT, MPI_COMM_WORLD);

	/*----------------------*/
	/* Loop for generations */
	/*----------------------*/
	for (k = 0; k < t; k++) {

		/*---------------------------------------------------------*/
		/* Send and receive arrays independently to find neighbors */
		/*---------------------------------------------------------*/
		MPI_Request      req[2];
		MPI_Status       status[2];
		MPI_Status       r_status[2];


		/* Send top and bottom row to previous and next process */
		if (processes > 1) {
			// Send top row to previous process
			MPI_Isend(&subboard[0], neighbor_size, MPI_CHAR,
			          prevproc, arraymsg, MPI_COMM_WORLD, req);

			// Send bottom row to next process
			MPI_Isend(&subboard[subboard_size - neighbor_size], neighbor_size, MPI_CHAR,
			          nextproc, arraymsg, MPI_COMM_WORLD, req + 1);

			//play current array
			for (i = 0; i < 2; i ++)
				for (j = 0; j < subcols; j ++)
					topborderboard[(i + 1) * subcols + j] = subboard[i * subcols + j];

			for (i = 0; i < 2; i ++)
				for (j = 0; j < subcols; j ++)
					bottomborderboard[i * subcols + j] = subboard[(subrows - 2 + i) * subcols + j];
		}


		play_in_parallel(subboard, newsubboard, subrows, subcols);


		if (processes > 1) {

			// Receive bottom neighbor(top row) from next process and play

			MPI_Recv(&bottomborderboard[2 * subcols], neighbor_size, MPI_CHAR,
			         nextproc, arraymsg, MPI_COMM_WORLD, &r_status[0]);
			play_in_parallel(bottomborderboard, newborderboard, 3, subcols);

			// Receive top neighbor(bottom row) from previous process and play

			MPI_Recv(&topborderboard[0], neighbor_size, MPI_CHAR,
			         prevproc, arraymsg, MPI_COMM_WORLD, &r_status[1]);
			play_in_parallel(topborderboard, newborderboard, 3, subcols);

			/* Add played borders to the chunk result */
			for (j = 0; j < subcols; j ++) {
				subboard[j] = topborderboard[1 * subcols + j];
				subboard[(subrows - 1) * subcols + j] = bottomborderboard[1 * subcols + j];
			}
		}


		/*---------------------------------------------*/
		/* Gather all sub-boards down to the ROOT task */
		/*---------------------------------------------*/
		if (disp)
			MPI_Gather(subboard, subboard_size, MPI_CHAR,
			           board, subboard_size, MPI_CHAR,
			           ROOT, MPI_COMM_WORLD);

		/*---------------------*/
		/* ROOT: Display board */
		/*---------------------*/
		if (disp && pid == ROOT)
			display_table (board, totalrows, totalcols);

	}

	/*----------------------------------------------------------------------*/
	/*                         STEP 3: Validate                             */
	/*----------------------------------------------------------------------*/

	/*------------------------------------*/
	/* Gather every sub-board in the ROOT */
	/*------------------------------------*/
	MPI_Gather(subboard, subboard_size, MPI_CHAR,
	           board, subboard_size, MPI_CHAR,
	           ROOT, MPI_COMM_WORLD);

	if (pid == ROOT)
		printf("PARALLEL GAME OVER after: %lf sec\n", MPI_Wtime() - parallelstarttime);


	if (pid == ROOT)
	{
		serialstarttime = MPI_Wtime();
		for (i = 0; i < t; i++)
			play_in_serial(validboard, newboard, totalrows, totalcols);
		printf("SERIAL GAME OVER after: %lf sec\n", MPI_Wtime() - serialstarttime);

		for (i = 0; i < totalrows; i++)
			for (j = 0; j < totalcols; j++)
				if (board[i * totalcols + j] != validboard[i * totalcols + j])
				{
					printf("FAIL!\n");
					return (1);
				}
	}

	/*----------------------------------------------------------------------*/
	/*                          STEP 4: Clean up                            */
	/*----------------------------------------------------------------------*/

	MPI_Finalize();

	return (0);
}
