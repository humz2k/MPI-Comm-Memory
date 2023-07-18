#main: src/mpi-comm-memory.cpp
#	mkdir -p lib
#	mpicxx src/mpi-comm-memory.cpp -c -fPIC -o lib/mpi-comm-memory.o

main: test/test.cpp test/test1.cpp
	mkdir -p build
	mpicxx test/test1.cpp -o test1.o -c -fPIC
	mpicxx test/test.cpp -o test2.o -c -fPIC
	mpicxx test1.o test2.o -o test.o