all:seda

seda: floristApp
	gcc -pthread floristApp.c -std=gnu99 -lm -o floristApp -lrt -lpthread -Wall -Wextra -pedantic

floristApp: floristApp.c
	gcc -c floristApp.c

clean: 
	rm -rf *.o floristApp
