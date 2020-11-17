all:seda

seda: server
	gcc -pthread server.c -std=gnu99 -lm -o server -lrt -lpthread -Wall -Wextra -pedantic
	gcc -pthread client.c -std=gnu99 -lm -o client -lrt -lpthread -Wall -Wextra -pedantic

server: server.c
	gcc -c server.c

client: client.c
	gcc -c client.c

clean: 
	rm -rf *.o server client
