#include "../include/mpi-comm-memory.hpp"

void test_func(){
    CommMemory<float>swap;
    swap.allocate(10);
}