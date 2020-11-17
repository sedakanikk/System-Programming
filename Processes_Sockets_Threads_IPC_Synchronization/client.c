#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <errno.h>
#include <stdlib.h> 
#include <sys/time.h>

int PORT=0, SOURCE_NODE=0, DESTINATION_NODE=0; 
int client_socket, read_size;
char *IP_ADDRESS;
struct sockaddr_in remote;

int main(int argc, char *argv[]) 
{
	int opt, ss=0;
	int arr[2];
	IP_ADDRESS = (char*)malloc(50*sizeof(char));

	while((opt = getopt(argc, argv, "apsd")) != -1) {	
		switch(opt)	{	
			case 'a':
				if(argv[optind]) {
					ss++;
					strcpy(IP_ADDRESS, argv[optind]);
				}
				break;
			case 'p':
				if(argv[optind]) {
					ss++;
					sscanf(argv[optind], "%d", &PORT);
				}
				break;
			case 's':
				if(argv[optind]) {
					ss++;
					sscanf(argv[optind], "%d", &SOURCE_NODE);
				}
				break;
			case 'd':
				if(argv[optind]) {
					ss++;
					sscanf(argv[optind], "%d", &DESTINATION_NODE);
				}
				break;
			default:
				errno = E2BIG;
				/* if some ekstra thing is given by command line, print message with errno	*/
				perror("The command line should be: ./client -a 127.0.0.1 -p PORT -s 768 -d 979 ");
				exit(EXIT_FAILURE);
				break;
		}	
	}
	if(argv[9]) {
		errno = E2BIG;
		/* if some ekstra thing is given by command line, print message with errno	*/
		perror("The command line should be: ./client -a 127.0.0.1 -p PORT -s 768 -d 979 ");
		exit(EXIT_FAILURE);
	}
	/* if the parameters is missing, prints usage error	*/
	if(ss != 4) {
		perror("\nThe command line should be: ./client -a 127.0.0.1 -p PORT -s 768 -d 979 ");
		exit(EXIT_FAILURE);
	}

	arr[0] = SOURCE_NODE;
	arr[1] = DESTINATION_NODE;

	//Created socket
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(client_socket == -1 ) {
		printf("Could not create socket\n");
		return 1;
	}
	memset(&remote, 0, sizeof(struct sockaddr_in));
	remote.sin_addr.s_addr = inet_addr(IP_ADDRESS); //Local Host
	remote.sin_family = AF_INET;
	remote.sin_port = htons(PORT);

	// connected to server
	printf("Client (%d) connecting to %s:%d\n",getpid(), IP_ADDRESS, PORT);
	int iRetval = connect(client_socket,(struct sockaddr*)&remote,sizeof(remote));
	if (iRetval < 0) {
		perror("connect failed.\n");
		return 1;
	}
	printf("Client (%d) connected and requesting a path from node %d to %d\n",getpid(), SOURCE_NODE, DESTINATION_NODE);

	struct timeval start, last;
	gettimeofday(&start, NULL);

	if (write(client_socket, arr, sizeof(arr)) < 0)
		perror("writing on stream socket"); 

	int begin=0, end=-1, number;
	read_size = read(client_socket, &end, sizeof(end));

	printf("Server’s response to (%d): ",  getpid());
	if(end > 0){
		int i;
		for(i=begin; i<end; ++i){
			read_size=read(client_socket, &number, sizeof(number));
			printf("%d", number);
			if(i != end-1)
				printf("->");
		}
		gettimeofday(&last, NULL);
		float time_spent = ((last.tv_sec * 1000000 + last.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))*1.0 / 1000000.0;
		printf(", arrived in %.1fseconds.\n", time_spent);
	}
	else {
		gettimeofday(&last, NULL);
		float time_spent = ((last.tv_sec * 1000000 + last.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))*1.0 / 1000000.0;
		printf("NO PATH, arrived in %.2fseconds, shutting down\n", time_spent);
	}

	close(client_socket);

	return 0; 
} 
