#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <time.h>
#include <unistd.h>

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

typedef struct client {
	char *name;
	float x_coord;
	float y_coord;
	char *flower_name;
	float distance;
}client;

typedef struct flowerist {
	char *name;
	float x_coord;
	float y_coord;
	float speed;
	char **flower_names;	
	int count_f;
	int *request_queue;
	int rear;
	int sales;
	int time;
	int flag;
}flowerist;

typedef struct statistics {
	char name[150];
	int time;
	int count_client;
}statistics;

/* global variables */
int ind, stay, count_flowers, flowerist_count, client_count, terminated_flowerist, stay_c, stay_f, flag = 0, flag_malloc=0, flag_file=0, flag_temp=0, fd_in;
char *input_filename, *buf, *temp;
char empty_char[100];

struct flowerist *flowerists;
struct client *clients;
struct statistics *sale_statistics;

pthread_cond_t cond_thread = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex;
pthread_mutex_t mutex_main;
pthread_mutex_t mutex_after;

/* functions */
void numberofFlowers(int filedes);
int numberofClients(int filedes);
int numberofFlorist(int filedes);
void readString(int filedes, char arr[]);
void pop(int index);
void *pool (void* arg);
void handler(int sig);

/* calculates number of flowers of given florist */
void numberofFlowers(int filedes) {
	int count = -1, i=0, j, m=0;
	ssize_t rbytes = 0; /* bytes read */
	size_t cbyte = sizeof(char);
	buf = (char*) malloc(120 * sizeof(char));
	temp = (char*) malloc(50 * sizeof(char));
	flag_temp = 1;

	/* check if file is empty */
	if(read(filedes, temp, cbyte) == -1) {
		perror("read:");
		exit(EXIT_FAILURE);
	}

	/* read string */
	while(1) {
		rbytes++;

		/* check if file is over */
		if(read(filedes, temp, cbyte) == -1) {
			perror("read:");
			exit(EXIT_FAILURE);
		}
		buf[i] = *temp;

		if ( (*temp == ',') )
			++count_flowers;

		/* check if string is over */
		if ( (*temp == '\n') ) {
			if(count_flowers != 0) {
				flowerists[m].count_f = count_flowers;
				m++;
			}
			count_flowers = 0;
			count++;
			for(j=0; j<=i; j++) 
				buf[j] = '\0';
			i = -1;
		}
		if ( (*temp == ' ') ) {
			
			if(buf[0] == 'c' && buf[1] == 'l' && buf[2] == 'i' && buf[3] == 'e' && buf[4] == 'n' && buf[5] == 't') {
				buf[++i] = '\0';
				break;
			}
		}
		i++;
	}
	free(temp);
	free(buf);
	flag_temp = 0;
}

/* calculates number of clients */
int numberofClients(int filedes) {
	int count = 1;
	int i=0, j, red;
	ssize_t rbytes = 0; /* bytes read */
	size_t cbyte = sizeof(char);
	buf = (char*) malloc(120 * sizeof(char));
	temp = (char*) malloc(50 * sizeof(char));
	flag_temp = 1;

	/* check if file is empty */
	if(read(filedes, temp, cbyte) == -1) {
		perror("read:");
		exit(EXIT_FAILURE);
	}

	/* read string */
	while(1) {
		rbytes++;

		/* check if file is over */
		red = read(filedes, temp, cbyte);
		if(red == 0)
			break;
		if(red == -1) {
			perror("read:");
			exit(EXIT_FAILURE);
		}
		buf[i] = *temp;

		if(buf[0] == 'c' && buf[1] == 'l' && buf[2] == 'i' && buf[3] == 'e' && buf[4] == 'n' && buf[5] == 't') {
			count++;
			for(j=0; j<i; j++) {
				buf[j] = '\0';
			}
		}
		/* check if string is over */
		if ( (*temp == '\n') ) {
			for(j=0; j<i; j++) {
				buf[j] = '\0';
			}
			i = -1;
		}
		i++;
	}
	free(temp);
	free(buf);
	flag_temp = 0;
	return count;
}

/* calculates number of florist */
int numberofFlorist(int filedes) {
	int count = -1;
	int i=0, j;
	ssize_t rbytes = 0; /* bytes read */
	size_t cbyte = sizeof(char);
	buf = (char*) malloc(120 * sizeof(char));
	temp = (char*) malloc(50 * sizeof(char));
	flag_temp = 1;

	/* check if file is empty */
	if(read(filedes, temp, cbyte) == -1) {
		perror("read:");
		exit(EXIT_FAILURE);
	}

	/* read string */
	while(1) {
		rbytes++;

		/* check if file is over */
		if(read(filedes, temp, cbyte) == -1) {
			perror("read:");
			exit(EXIT_FAILURE);
		}
		buf[i] = *temp;

		/* check if string is over */
		if ( (*temp == '\n') ) {
			count++;
			for(j=0; j<=i; j++) 
				buf[j] = '\0';
			i = -1;
		}
		if ( (*temp == ' ') ) {
			
			if(buf[0] == 'c' && buf[1] == 'l' && buf[2] == 'i' && buf[3] == 'e' && buf[4] == 'n' && buf[5] == 't') {
				buf[++i] = '\0';
				break;
			}
		}
		i++;
	}
	free(temp);
	free(buf);
	flag_temp = 0;
	return count;
}

/* reads all the file */
void readString(int filedes, char arr[]) {
	int i;
	ssize_t rbytes = 0; /* bytes read */
	size_t cbyte = sizeof(char);
	buf = (char*) malloc(120 * sizeof(char));
	temp = (char*) malloc(50 * sizeof(char));
	flag_temp = 1;

	/* check if file is empty */
	if(read(filedes, temp, cbyte) == -1) {
		perror("read:");
		exit(EXIT_FAILURE);
	}
	ind++;

	/* eat spaces */
	while ( (*temp == ' ') || (*temp == '\n') || (*temp == '\t') || (*temp == '(') || (*temp == ')') || (*temp == ',') || (*temp == ';') || (*temp == ':') ) {
		if(read(filedes, temp, cbyte) == -1) {
			perror("read:");
			exit(EXIT_FAILURE);
		}
	}

	/* read string */
	i=0;
	while(1) {
		buf[i] = *temp;
		rbytes++;

		/* check if file is over */
		if(read(filedes, temp, cbyte) == -1) {
			perror("read:");
			exit(EXIT_FAILURE);
		}

		/* check if string is over */
		if ( (*temp == ' ') || (*temp == '\n') || (*temp == '\t') || (*temp == '(') || (*temp == ')') || (*temp == ',') || (*temp == ';') || (*temp == ':') ) {
			if(*temp == '\n') {
				/* keeps the index of \n */
				stay = ind;
			}
			break;
		}
		i++;
	}
	buf[++i] = '\0';
	strcpy(arr, buf);
	
	free(temp);
	free(buf);
	flag_temp = 0;
}

/* queue operation to change the first element of the queue */
void pop(int index) {
	int i;
	if(flowerists[index].rear <= 0) {
		flowerists[index].rear = -1;
		return;
	}
	/* decrease the number of client which in queue */
	(flowerists[index].rear)--;
	if(flowerists[index].rear == 0) {
		flowerists[index].rear = -1;
		return;
	}
	for(i=0; i<client_count-1; i++) {
		flowerists[index].request_queue[i] = flowerists[index].request_queue[i+1];
	}
	flowerists[index].request_queue[client_count-1] = 0;
}

/* thread function */
void *pool (void* arg) {
	int index_flowerist, index_client, st, res;
	float resssss;
	index_flowerist = *(int*)arg;
	/* locks with mutex */
	if(pthread_mutex_lock(&mutex) != 0) {
		perror("pthread_mutex_lock->mutex:");
		exit(EXIT_FAILURE);
	}
	/* executes while there is any client in queue */
	while(flowerists[index_flowerist].rear != 0 && flowerists[index_flowerist].rear != -1) {

		if(pthread_cond_wait(&cond_thread, &mutex) != 0) {
			perror("pthread_cond_wait->cond_thread:");
			exit(EXIT_FAILURE);
		}
		/* takes the index of the first client of the queue */
		index_client = flowerists[index_flowerist].request_queue[0];
		/* pops the first client */
		pop(index_flowerist);
		flowerists[index_flowerist].flag=1;
		st=(rand()%250)+1;
		resssss = (clients[index_client].distance / flowerists[index_flowerist].speed);
		res = resssss + (st);
		sale_statistics[index_flowerist+1].time += res;
		/*usleep((int)res*1000);*/
		printf("Florist %s has delivered a %s to %s in %dms\n", flowerists[index_flowerist].name, clients[index_client].flower_name, clients[index_client].name, res);
	}
	if(pthread_mutex_unlock(&mutex) != 0) {
		perror("pthread_mutex_unlock->mutex");
		exit(EXIT_FAILURE);
	}

	if(pthread_mutex_lock(&mutex_after) != 0) {
		perror("pthread_mutex_lock->mutex_after");
		exit(EXIT_FAILURE);
	}
	/* terminates if the florist has no more client in queue */
	while(flowerists[index_flowerist].rear == -1) {
		terminated_flowerist++;
		break;
	}
	if(flowerists[index_flowerist].flag != 1)
			terminated_flowerist++;
	if(pthread_mutex_unlock(&mutex_after) != 0) {
		perror("pthread_mutex_unlock->mutex_after:");
		exit(EXIT_FAILURE);	
	}
	sale_statistics[index_flowerist+1].count_client = flowerists[index_flowerist].sales;
	strcpy(sale_statistics[index_flowerist+1].name, flowerists[index_flowerist].name);
	return (void*)sale_statistics;
}

/* handler for signal SIGINT  */
void handler(int sig) {
	int i, j;
	if(sig == SIGINT) {
		printf("Welcome to SIGINT handler!\n");
		/* checks all the allocates to free */
		if(flag_temp == 1) {
			free(temp);
			free(buf);
		}
		if(flag_file == 1 && flag == 0) {
			free(input_filename);
			if(close(fd_in) == -1) {
				perror("close:");
				exit(EXIT_FAILURE);
			}
		}
		if(flag_malloc == 1 && flag == 0) {

			for(i=0; i<flowerist_count; i++) {
				for(j=0; j<flowerists[i].count_f; j++) 
					free(flowerists[i].flower_names[j]);
				free(flowerists[i].name);
				free(flowerists[i].flower_names);
				free(flowerists[i].request_queue);
			}
			free(flowerists);

			for(i=0; i<client_count; i++) {
				free(clients[i].name);
				free(clients[i].flower_name);
			}
			free(clients);
			free(sale_statistics);
		}
	}
	exit(0);
}

/* main thread */
int main(int argc, char* argv[])
{
	int ss=0, i, j, k, opt, begin, ind_florists;
	float chebyshev_distance=0.0; 
	srand(time(NULL));

	/* setted to block SIGINT */
	struct sigaction sa;
	sigset_t blockMask;
	if(sigemptyset(&sa.sa_mask) == -1) {
		perror("sigemptyset:");
		exit(EXIT_FAILURE);
	}
	sa.sa_flags = 0;
	sa.sa_handler = handler;
	if(sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction:");
		exit(EXIT_FAILURE);
	}
	if(sigemptyset(&blockMask) == -1) {
		perror("sigemptyset:");
		exit(EXIT_FAILURE);
	}
	if(sigaddset(&blockMask, SIGINT) == -1) {
		perror("sigaddset:\n");
		exit(EXIT_FAILURE);
	}

	ind=-1;
	count_flowers=0;
	terminated_flowerist = 0;

	/* signal SIGINT is blocked */
	if (sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1) {
		perror("sigprocmask:");
		exit(EXIT_FAILURE);
	}
	flag_file=1;
	input_filename = (char*)malloc(30*sizeof(char));
	/* signal SIGINT is unblocked */
	if (sigprocmask(SIG_UNBLOCK, &blockMask, NULL) == -1) {
		perror("sigprocmask:");
		exit(EXIT_FAILURE);
	}

	while((opt = getopt(argc, argv, "i")) != -1) {	
		switch(opt)	{	
			case 'i':
				if(argv[optind]) {
					ss=1;
					strcpy(input_filename, argv[optind]);
				}
				break;
			default:
				errno = E2BIG;
				/* if some ekstra thing is given by commend line, print message with errno  */
				perror("The commond line should be: ./floristApp -i data.dat ");
				exit(EXIT_FAILURE);
				break;
		}	
	}
	if(argv[3]) {
		errno = E2BIG;
		/* if some ekstra thing is given by commend line, print message with errno  */
		perror("The commond line should be: ./floristApp -i data.dat ");
		exit(EXIT_FAILURE);
	}

	/* if the parameters is missing, prints usage error  */
	if(ss == 0) {
		errno = ENOENT;
		perror("The file is not given \nThe commond line should be: ./floristApp -i data.dat ");
		exit(EXIT_FAILURE);
	}

	printf("Florist application initializing from file: %s\n", input_filename);

	/* opens file  */
	fd_in = open(input_filename, O_RDONLY);
	if(fd_in == -1) {
		errno = EEXIST;
		perror("open");
		exit(EXIT_FAILURE);
	}

	/* calculates counts of florist and client */
	flowerist_count = numberofFlorist(fd_in);
	client_count = numberofClients(fd_in);

	/* signal SIGINT is blocked */
	if (sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1) {
		perror("sigprocmask:");
		exit(EXIT_FAILURE);
	}
	flag_malloc=1;

	flowerists = (struct flowerist*) malloc(flowerist_count * sizeof(struct flowerist));
	if(lseek(fd_in,0,SEEK_SET) == -1) {
		perror("lseek: ");
		exit(EXIT_FAILURE);
	}
	numberofFlowers(fd_in);

	/* allocates some spaces */
	for(i=0; i<flowerist_count; i++) {
		flowerists[i].name = (char*) malloc(150 * sizeof(char));
		flowerists[i].time = 0.0;
	}
	for(i=0; i<flowerist_count; i++) {
		flowerists[i].flower_names = (char**) malloc((flowerists[i].count_f) * sizeof(char*));
	}
	for(i=0; i<flowerist_count; i++) {
		for(j=0; j<flowerists[i].count_f; j++) 
			flowerists[i].flower_names[j] = (char*) malloc(50 * sizeof(char));
	}
	for(i=0; i<flowerist_count; ++i) {
		flowerists[i].rear = 0;
	}
	for(i=0; i<flowerist_count; i++) {
		flowerists[i].request_queue = (int*) malloc(client_count * sizeof(int));
	}
	clients = (struct client*) malloc(client_count * sizeof(struct client));
	for(i=0; i<client_count; i++) {
		clients[i].name = (char*) malloc(150*sizeof(char));
		clients[i].flower_name = (char*) malloc(50*sizeof(char));
	}
	sale_statistics = (struct statistics*) malloc((flowerist_count+1) * sizeof(struct statistics));
	for(i=0; i<=flowerist_count; i++) {
		sale_statistics[i].time = 0;
	}
	/* signal SIGINT is unblocked */
	if (sigprocmask(SIG_UNBLOCK, &blockMask, NULL) == -1) {
		perror("sigprocmask:");
		exit(EXIT_FAILURE);
	}

	/* go to the top of the file */
	if(lseek(fd_in,0,SEEK_SET) == -1) {
		perror("lseek: ");
        exit(EXIT_FAILURE);
	}

	ind_florists = 0;
	begin = 0;
	stay = 0;
	/* all the florists and their informaions setted */
	while(begin == stay) { 
		if(ind_florists<flowerist_count) {
			readString(fd_in, empty_char);
			strcpy(flowerists[ind_florists].name, empty_char);
			readString(fd_in, empty_char);
			sscanf(empty_char, "%f", &(flowerists[ind_florists].x_coord));
			readString(fd_in, empty_char);			
			sscanf(empty_char, "%f", &(flowerists[ind_florists].y_coord));
			readString(fd_in, empty_char);
			sscanf(empty_char, "%f", &(flowerists[ind_florists].speed));

			for(i=0; i<flowerists[ind_florists].count_f; i++) {
				readString(fd_in, empty_char);
				strcpy(flowerists[ind_florists].flower_names[i], empty_char);
			}
			begin = stay;
			++ind_florists;
		}
		else
			break;
	}

	/* all the clients and their informaions setted */
	for(i=0; i<client_count; i++) {
		readString(fd_in, empty_char);
		strcpy(clients[i].name, empty_char);
		readString(fd_in, empty_char);
		sscanf(empty_char, "%f", &(clients[i].x_coord));
		readString(fd_in, empty_char);
		sscanf(empty_char, "%f", &(clients[i].y_coord));
		readString(fd_in, empty_char);
		strcpy(clients[i].flower_name, empty_char);
	}

	k=0, stay_c=0, stay_f=0;
	while(k<client_count) {
		/* initialized as max of float */
		clients[k].distance = FLT_MAX;

		for(i=0; i<flowerist_count; ++i) {
			for(j=0; j<flowerists[i].count_f; ++j) {

				if(strcmp(clients[k].flower_name, flowerists[i].flower_names[j]) == 0) {
					chebyshev_distance = MAX(fabs(flowerists[i].x_coord - clients[k].x_coord), fabs(flowerists[i].y_coord - clients[k].y_coord));

					if(clients[k].distance > chebyshev_distance) {
						clients[k].distance = chebyshev_distance;
						/* if the distance is bigger than calculated keep the index of the calculated distance */
						stay_f = i;
						stay_c = k;
					}
				}
			}
		}
		/* keeps index of client */
		flowerists[stay_f].request_queue[flowerists[stay_f].rear] = stay_c;
		++(flowerists[stay_f].rear);
		++k;
	}
	/* keeps the number of client sor each florist */
	for(j=0; j<flowerist_count; ++j) {
		flowerists[j].sales = flowerists[j].rear;
	}

	/* signal SIGINT is blocked */
	if (sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1) {
		perror("sigprocmask:");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_init(&mutex, NULL) != 0) {
		perror("pthread_mutex_init->mutex");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_init(&mutex_main, NULL) != 0) {
		perror("pthread_mutex_init->mutex_main");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_init(&mutex_after, NULL) != 0) {
		perror("pthread_mutex_init->mutex_after");
		exit(EXIT_FAILURE);
	}

	/* flowerist threads  */
	int flowerist_ids[flowerist_count+1];
	pthread_t  flowerist_threads[flowerist_count+1];

	/* flowerist threads created  */
	for(i=0; i<flowerist_count; i++) {
		flowerist_ids[i] = i;
		if (pthread_create(&flowerist_threads[i], NULL, pool, &flowerist_ids[i]) != 0) {
			perror("pthread_create->flowerist_threads\nInsufficient resources to create thread");
			exit(EXIT_FAILURE);
		}
	}
	printf("%d florists have been created\n", flowerist_count);
	printf("Processing requests\n");

	/* sends signal to threads will work */
	if(pthread_mutex_lock(&mutex_main) != 0) {
		perror("pthread_mutex_lock->mutex_main:");
		exit(EXIT_FAILURE);
	}
	while(terminated_flowerist < flowerist_count) {
		if(pthread_cond_signal(&cond_thread) != 0) {
			perror("pthread_cond_signal->cond_thread:");
			exit(EXIT_FAILURE);
		}
	}
	if(pthread_mutex_unlock(&mutex_main) != 0) {
		perror("pthread_mutex_unlock->mutex_main:");
		exit(EXIT_FAILURE);
	}
	printf("All requests processed.\n");

	/* waits until all the threads are terminated */
	for (i = 0; i < flowerist_count; ++i) {
		if(pthread_join(flowerist_threads[i], (void*)sale_statistics) != 0) {
			perror("pthread_join->flowerist_threads[i]");
			exit(EXIT_FAILURE);
		}
		printf("%s closing shop.\n", flowerists[i].name);
	}

	/* cleans all the mutexes and condition variable */
	if(pthread_mutex_destroy(&mutex) != 0) {
		perror("pthread_mutex_destroy->mutex:");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_destroy(&mutex_main) != 0) {
		perror("pthread_mutex_destroy->mutex_main:");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_destroy(&mutex_after) != 0) {
		perror("pthread_mutex_destroy->mutex_after:");
		exit(EXIT_FAILURE);
	}
	if(pthread_cond_destroy(&cond_thread) != 0) {
		perror("pthread_cond_destroy->cond_thread:");
		exit(EXIT_FAILURE);
	}

	/* signal SIGINT is unblocked */
	if (sigprocmask(SIG_UNBLOCK, &blockMask, NULL) == -1) {
		perror("sigprocmask:");
		exit(EXIT_FAILURE);
	}
	
	/* prints the information of statistics */
	printf("Sale statistics for today:\n");
	printf("-------------------------------------------------\n");

	printf("Florist%-9c# of sales%-9cTotal time\n", ' ', ' ');
	printf("-------------------------------------------------\n");
	for(i=1; i<=flowerist_count; i++) {
		printf("%-9s \t%-9d %-9c%dms\n", sale_statistics[i].name, sale_statistics[i].count_client, ' ', sale_statistics[i].time);
	}
	printf("-------------------------------------------------\n");

	/* closes file and frees all the resources */
	/* signal SIGINT is blocked */
	if (sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1) {
		perror("sigprocmask:");
		exit(EXIT_FAILURE);
	}
	/* if cleans all the thing set flag as 1 for handler to see */
	flag=1;
	if(close(fd_in) == -1) {
		perror("close:");
		exit(EXIT_FAILURE);
	}
	free(input_filename);
	for(i=0; i<flowerist_count; i++) {
		for(j=0; j<flowerists[i].count_f; j++) 
			free(flowerists[i].flower_names[j]);
		free(flowerists[i].name);
		free(flowerists[i].flower_names);
		free(flowerists[i].request_queue);
	}
	free(flowerists);
	for(i=0; i<client_count; i++) {
		free(clients[i].name);
		free(clients[i].flower_name);
	}
	free(clients);
	free(sale_statistics);
	
	/* signal SIGINT is unblocked */
	if (sigprocmask(SIG_UNBLOCK, &blockMask, NULL) == -1) {
		perror("sigprocmask:");
		exit(EXIT_FAILURE);
	}

	return 0;
}