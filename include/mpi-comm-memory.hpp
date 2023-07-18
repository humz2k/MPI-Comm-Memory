#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_BLOCKS 8
#define DEFAULT_BLOCKSIZE 8

template<class T>
class CommMemory{
    public:
        int comm_rank;
        int comm_size;
        T* local_memory;
        T* cache;
        bool* valid;
        int* id;
        int nelements;
        bool allocated;
        MPI_Comm comm;
        int localBlocks;
        int cacheBlocks;
        int blockSize;

        void init(int n_blocks_, int blockSize_, MPI_Comm comm);

        CommMemory(int n_blocks_, int blockSize_, MPI_Comm comm_);

        CommMemory(int n_blocks_, int blockSize_);

        CommMemory();

        void allocate(int n);

        void deallocate();

        T& operator[](int index);

        T operator()(int rank, int index);

        ~CommMemory();

        operator T*();
};


template<class T>
CommMemory<T>::CommMemory(){
    init(DEFAULT_BLOCKS,DEFAULT_BLOCKSIZE,MPI_COMM_WORLD);
}

template<class T>
CommMemory<T>::CommMemory(int n_blocks_, int blockSize_){
    init(n_blocks_,blockSize_,MPI_COMM_WORLD);
}

template<class T>
CommMemory<T>::CommMemory(int n_blocks_, int blockSize_, MPI_Comm comm_){

    init(n_blocks_,blockSize_,comm_);
}

template<class T>
void CommMemory<T>::init(int n_blocks_, int blockSize_, MPI_Comm comm_){

    comm = comm_;

    MPI_Comm_rank(comm,&comm_rank);
    MPI_Comm_size(comm,&comm_size);
    
    cacheBlocks = n_blocks_;
    blockSize = blockSize_;

    allocated = false;
}

template<class T>
void CommMemory<T>::allocate(int n){

    nelements = n;
    localBlocks = (nelements + (blockSize-1))/blockSize;
    local_memory = (T*)malloc(sizeof(T)*localBlocks*blockSize);
    cache = (T*)malloc(sizeof(T)*cacheBlocks*blockSize);
    valid = (bool*)malloc(sizeof(bool)*cacheBlocks);
    for (int i = 0; i < cacheBlocks; i++){
        valid[i] = false;
    }
    id = (int*)malloc(sizeof(int)*cacheBlocks);
    allocated = true;

}

template<class T>
void CommMemory<T>::deallocate(){
    if (allocated){
        free(local_memory);
        free(cache);
        free(valid);
        free(id);
    }
    allocated = false;
}

template<class T>
CommMemory<T>::~CommMemory(){
    if (allocated){
        deallocate();
    }
}

template<class T>
T& CommMemory<T>::operator[](int index){
    return local_memory[index];
}

template<class T>
CommMemory<T>::operator T*(){
    return local_memory;
}

template<class T>
T CommMemory<T>::operator()(int rank, int index){
    int blockID = rank * localBlocks + (index/blockSize);
    int thisCacheBlock = blockID % cacheBlocks;
    int found = 0;
    T out;
    int torecv[comm_size];
    for (int i = 0; i < comm_size; i++){
        torecv[i] = -1;
    }
    if (rank == comm_rank){
        found = 1;
        out = local_memory[index];
    } else if ((valid[thisCacheBlock]) && (id[thisCacheBlock] == blockID)){
        found = 1;
        out = cache[thisCacheBlock * blockSize + (index % blockSize)];
    } else{
        torecv[rank] = index/blockSize;
    }
    int done = 0;
    MPI_Allreduce(&found,&done,1,MPI_INT,MPI_LAND,comm);
    if (done){
        return out;
    }

    int tosend[comm_size];
    for (int i = 0; i < comm_size; i++){
        tosend[i] = -1;
    }
    for (int i = 0; i < comm_size; i++){
        if (i == comm_rank)continue;
        MPI_Request request;
        MPI_Isend(&torecv[i],1,MPI_INT,i,0,comm,&request);
        MPI_Request_free(&request);     
    }
    for (int i = 0; i < comm_size; i++){
        if (i == comm_rank)continue;
        MPI_Recv(&tosend[i],1,MPI_INT,i,0,comm,MPI_STATUS_IGNORE);
    }

    //MPI_Alltoall(torecv,1,MPI_INT,tosend,1,MPI_INT,comm);

    for (int i = 0; i < comm_size; i++){
        if (i == comm_rank)continue;
        if (tosend[i] != -1){
            MPI_Request request;
            int block = tosend[i];
            int globalID = comm_rank * localBlocks + block;
            //printf("rank %d send %d to %d\n",comm_rank,globalID,i);
            MPI_Isend(&local_memory[block*blockSize],blockSize*sizeof(T),MPI_BYTE,i,0,comm,&request);
            MPI_Request_free(&request);
        }
    }
    for (int i = 0; i < comm_size; i++){
        if (i == comm_rank)continue;
        if (torecv[i] != -1){
            int block = torecv[i];
            int globalID = i * localBlocks + block;
            int localCacheBlock = globalID % cacheBlocks;
            //printf("rank %d recv %d from %d\n",comm_rank,localCacheBlock,i);
            MPI_Recv(&cache[localCacheBlock*blockSize],blockSize*sizeof(T),MPI_BYTE,i,0,comm,MPI_STATUS_IGNORE);
            valid[localCacheBlock] = true;
            id[localCacheBlock] = globalID;
        }
    }

    if (!found){
        out = cache[thisCacheBlock * blockSize + (index % blockSize)];
    }

    return out;
}