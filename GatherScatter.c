#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#define N 10

	/*int i;
	char *buffer;
	buffer = (unsigned char *)malloc(sizeof(unsigned char) * N);
	
	for(i = 0; i < N; i++){
		buffer[i] = (unsigned char)i;
	}

      // Initialize the MPI environment
      MPI_Init(NULL, NULL);

      // Get the number of processes
      int world_size;
      MPI_Comm_size(MPI_COMM_WORLD, &world_size);

      // Get the rank of the process
      int world_rank;
      MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
      
      char *buffer2;
      buffer2 = (unsigned char *)malloc(sizeof(unsigned char) * N/world_size);

		printf("%d ", world_rank);
      MPI_Scatter(&buffer, N/world_size, MPI_FLOAT, 
                  &buffer2, N/world_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
                  
      for(i = 0; i < N/world_size; i++){
		buffer2[i] *= 2;
      }
      
      MPI_Gather(&buffer, N/world_size, MPI_INT, &buffer2, N/world_size, MPI_INT, 0, MPI_COMM_WORLD);          
      
      
      for(i = 0; i < N; i++){
		printf("%d ", buffer[i]);
      }
      
      // Finalize the MPI environment.
      MPI_Finalize();
      return 0;*/
      
int main(int argc, char **argv) {
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char globaldata[N];/*wants to declare array this way*/
    char localdata[N / size + 1];/*without using pointers*/

    int i;
    if (rank == 0) {

        for (i = 0; i < N; i++){
            globaldata[i] = (unsigned char)i; //1
        }

        printf("1. Processor %d has data: ", rank);
        for (i = 0; i < N; i++){
            printf("%d ", globaldata[i]);
        }
        printf("\n");
    }

    MPI_Scatter(globaldata, N / size + 1, MPI_CHAR, &localdata, N / size + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    
    printf("2. Processor %d has data ", rank);
    for(i = 0; i < N / size + 1; i++){
    	localdata[i] *= 10;
    	printf("%d ", localdata[i]);
    }
    printf("\n"); 


    MPI_Gather(&localdata, N / size + 1, MPI_CHAR, globaldata, N / size + 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("3. Processor %d has data: ", rank);
        for (i = 0; i < N; i++)
            printf("%d ", globaldata[i]);
        printf("\n\n");
    }


    MPI_Finalize();
    return 0;
}

