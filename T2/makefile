make: t2.c
	gcc -o t2 -c t2.c -fopenmp -lm

teste: t2.c
	gcc -o t2t -c t2_teste.c -fopenmp -lm

run:
	./t2 < teste


run_teste:
	./t2t < teste