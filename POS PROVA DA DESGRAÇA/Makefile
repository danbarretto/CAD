make: multmat.c
	@ mpicc multmat.c -o exec -fopenmp

run:
	@ mpirun -np 2 exec

run3:
	@ mpirun -np 3 exec