/*
  Daniel Sá Barretto Prado Garcia 103734344
  Felipe Guilermmo Santuche Moleiro 10724010
  Laura Alves de Jesus 10801180
  Tiago Marino Silva 10734748
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int  main(int argc, char *argv[])  
{
	int npes, myrank, src, dest, msgtag;
	int token;
	MPI_Status status;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &npes);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	
   	msgtag = 1;
	if ( myrank == 0) {
        src = npes - 1;
        dest = 1;
		token = 0;
		//Send token to first destination on tokenring
		MPI_Send(&token, 1, MPI_INT, dest, msgtag, MPI_COMM_WORLD);
		printf("process %d sent %d\n", myrank, token);
		msgtag = npes;
		//receive token from last position on tokenring
		MPI_Recv(&token, 1, MPI_INT, src, msgtag, MPI_COMM_WORLD, &status);
		printf("process %d received %d from %d\n", myrank, token, src);
	}
	else {

		// Receive from previous
		src = myrank - 1;
        // Send to next on queue
        dest = (myrank + 1) % npes;
		// Message tag is equal to rank
		msgtag = myrank;

		// Receive integer
        MPI_Recv(&token, 1, MPI_INT, src, msgtag, MPI_COMM_WORLD, &status);
		printf("process %d received %d from %d\n", myrank, token, src);
		// Count +1 and convert to string
		token++;
        // Send to next on line with next tag
		msgtag = myrank + 1;
		printf("process %d sent %d to %d\n", myrank, token, dest);
		fflush(stdout);
		MPI_Send(&token, 1, MPI_INT, dest, msgtag, MPI_COMM_WORLD);
    }

	fflush(0);
	
	return !(MPI_Finalize() == MPI_SUCCESS);
}
