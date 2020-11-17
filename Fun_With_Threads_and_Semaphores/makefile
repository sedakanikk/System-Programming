all:seda

seda: program
	gcc -Wall -Wextra -pedantic program.c -o program -pthread

program: program.c
	gcc -c program.c

clean: 
	rm -rf *.o program


