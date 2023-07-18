#include "../include/mpi-comm-memory.hpp"
#include "test_include.hpp"

int main(){
    MPI_Init(NULL,NULL);   
    CommMemory<float>swap;
    swap.allocate(10);

    for (int i = 0; i < 10; i++){
        swap[i] = swap.comm_rank * 10 + i;
    }
    double start,end;
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
    if(swap.comm_rank==0)printf("Test\n");
    float test = swap((swap.comm_rank + 1)%swap.comm_size,0);
    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();
    if(swap.comm_rank==0)printf("t = %g\n",end-start);

    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
    if(swap.comm_rank==0)printf("Test2\n");
    float test2 = swap((swap.comm_rank + 1)%swap.comm_size,1);
    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();
    if(swap.comm_rank==0)printf("t = %g\n",end-start);

    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
    if(swap.comm_rank==0)printf("Test3\n");
    float test3 = swap(0,1);
    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();
    if(swap.comm_rank==0)printf("t = %g\n",end-start);

    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
    if(swap.comm_rank==0)printf("Test4\n");
    float test4 = swap(0,2);
    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();
    if(swap.comm_rank==0)printf("t = %g\n",end-start);

    MPI_Barrier(MPI_COMM_WORLD);
    if(swap.comm_rank==0)printf("Done\n");

    MPI_Barrier(MPI_COMM_WORLD);
    for (int i = 0; i < swap.comm_size; i++){
        if (i == swap.comm_rank){
            printf("rank %d -> test: %g, test2: %g, test3: %g, test4: %g\n",swap.comm_rank,test,test2,test3, test4);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    //for (int i = 0; i < 10; i++){
    //    printf("Rank %d: %d = %g\n",swap.comm_rank,i,swap[i]);
    //}

    swap.deallocate();
    MPI_Finalize();
    return 0;
}