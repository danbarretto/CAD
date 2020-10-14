make:
	gcc t1.c -o t1 -fopenmp
run:
	./t1 $(tam)
test:
	if [-f ./test.out]; then rm test.out; fi
	for number in 10 100 1000 10000 100000 1000000 10000000 100000000 1000000000; do \
		./t1 $$number >> test.out; \
		echo "\n" >> test.out; \
	done