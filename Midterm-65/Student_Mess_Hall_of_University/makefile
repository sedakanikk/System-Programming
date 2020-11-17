all:seda

seda: program.o
	gcc program.o -lm -o program -pthread -lrt

program.o: program.c
	gcc -c program.c

clean: 
	rm -rf *.o program
