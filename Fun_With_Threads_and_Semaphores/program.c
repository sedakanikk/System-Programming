#include <errno.h>
#include <fcntl.h>
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

void* chef(void* arg);
void* pusher(void* arg);
/* pusher mutex represents when a chef has the items they need */
/* if the gullac is ready, chef will know this and can be obtain gullac by wholesaler_ready */
/* the others is ingredients */
sem_t mutex_pusher;
sem_t wholesaler_ready;
sem_t milk_flour;
sem_t milk_walnuts;
sem_t milk_sugar;
sem_t flour_walnuts;
sem_t flour_sugar;
sem_t walnuts_sugar;
/* This is a data structure of read ingredients names from file */
char* onechar;

/* function for chef threads */
void* chef(void* arg)
{
	int chef_id;
	chef_id = *(int*) arg;
	/* seed our random number since we will be using random numbers */
	srand(time(NULL));
	while (true) {
		/* determines ids of chefs and executes according to chefs */
		if(chef_id==0){
			printf("chef%d is waiting for milk and flour\n", chef_id+1);
			/* chef1 has walnuts and sugar, if the ingredients are milk and flour, chef1 takes them */
			if(sem_wait(&milk_flour) == -1) {
				perror("sem_wait->milk_flour");
				exit(EXIT_FAILURE);
			}
			/* chef1 breaks if there is no more milk and flour  */
			if(onechar[0]=='-') 
				break;

			printf("chef%d has taken the milk\n", chef_id+1);
			printf("chef%d has taken the flour\n", chef_id+1);
			/* chef1 is making gullac now */
			printf("chef%d is preparing the dessert\n", chef_id+1);
			sleep(rand() % 5 + 1);
			printf("chef%d has delivered the dessert to the wholesaler\n", chef_id+1);
			if(sem_post(&wholesaler_ready) == -1) {
				perror("sem_post->wholesaler_ready");
				exit(EXIT_FAILURE);
			}
		}
		else if(chef_id==1){
			printf("chef%d is waiting for milk and walnuts\n", chef_id+1);
			/* chef2 has flour and sugar, if the ingredients are milk and walnuts, chef2 takes them */
			if(sem_wait(&milk_walnuts) == -1) {
				perror("sem_wait->milk_walnuts");
				exit(EXIT_FAILURE);
			}
			/* chef2 breaks if there is no more milk and walnuts */
			if(onechar[0]=='-') 
				break;

			printf("chef%d has taken the milk\n", chef_id+1);
			printf("chef%d has taken the walnuts\n", chef_id+1);
			printf("chef%d is preparing the dessert\n", chef_id+1);
			/* chef2 is making gullac now */
			sleep(rand() % 5 + 1);
			printf("chef%d has delivered the dessert to the wholesaler\n", chef_id+1);
			if(sem_post(&wholesaler_ready) == -1) {
				perror("sem_post->wholesaler_ready");
				exit(EXIT_FAILURE);
			}
		}
		else if(chef_id==2){
			printf("chef%d is waiting for milk and sugar\n", chef_id+1);
			/* chef3 has flour and walnuts, if the ingredients are milk and sugar, chef3 takes them */
			if(sem_wait(&milk_sugar) == -1) {
				perror("sem_wait->milk_sugar");
				exit(EXIT_FAILURE);
			}
			/* chef3 breaks if there is no more milk and sugar */
			if(onechar[0]=='-') 
				break;

			printf("chef%d has taken the milk\n", chef_id+1);
			printf("chef%d has taken the sugar\n", chef_id+1);
			printf("chef%d is preparing the dessert\n", chef_id+1);
			/* chef3 is making gullac now */
			sleep(rand() % 5 + 1);
			printf("chef%d has delivered the dessert to the wholesaler\n", chef_id+1);
			if(sem_post(&wholesaler_ready) == -1) {
				perror("sem_post->wholesaler_ready");
				exit(EXIT_FAILURE);
			}
		}
		else if(chef_id==3){
			printf("chef%d is waiting for flour and walnuts\n", chef_id+1);
			/* chef4 has milk and sugar, if the ingredients are flour and walnuts, chef4 takes them */
			if(sem_wait(&flour_walnuts) == -1) {
				perror("sem_wait->flour_walnuts");
				exit(EXIT_FAILURE);
			}
			/* chef4 breaks if there is no more flour and walnuts */
			if(onechar[0]=='-') 
				break;

			printf("chef%d has taken the flour\n", chef_id+1);
			printf("chef%d has taken the walnuts\n", chef_id+1);
			printf("chef%d is preparing the dessert\n", chef_id+1);
			/* chef4 is making gullac now */
			sleep(rand() % 5 + 1);
			printf("chef%d has delivered the dessert to the wholesaler\n", chef_id+1);
			if(sem_post(&wholesaler_ready) == -1) {
				perror("sem_post->wholesaler_ready");
				exit(EXIT_FAILURE);
			}
		}
		else if(chef_id==4){
			printf("chef%d is waiting for flour and sugar\n", chef_id+1);
			/* chef5 has milk and walnuts, if the ingredients are flour and sugar, chef5 takes them */
			if(sem_wait(&flour_sugar) == -1) {
				perror("sem_wait->flour_sugar");
				exit(EXIT_FAILURE);
			}
			/* chef5 breaks if there is no more flour and sugar */
			if(onechar[0]=='-') 
				break;

			printf("chef%d has taken the flour\n", chef_id+1);
			printf("chef%d has taken the sugar\n", chef_id+1);
			printf("chef%d is preparing the dessert\n", chef_id+1);
			/* chef5 is making gullac now */
			sleep(rand() % 5 + 1);
			printf("chef%d has delivered the dessert to the wholesaler\n", chef_id+1);
			if(sem_post(&wholesaler_ready) == -1) {
				perror("sem_post->wholesaler_ready");
				exit(EXIT_FAILURE);
			}
		}
		else if(chef_id==5){ 
			printf("chef%d is waiting for walnuts and sugar\n", chef_id+1);
			/* chef6 has milk and flour, if the ingredients are walnuts and sugar, chef6 takes them */
			if(sem_wait(&walnuts_sugar) == -1) {
				perror("sem_wait->walnuts_sugar");
				exit(EXIT_FAILURE);
			}
			/* chef6 breaks if there is no more walnuts and sugar */
			if(onechar[0]=='-') 
				break;

			printf("chef%d has taken the walnuts\n", chef_id+1);
			printf("chef%d has taken the sugar\n", chef_id+1);
			printf("chef%d is preparing the dessert\n", chef_id+1);
			/* chef6 is making gullac now */
			sleep(rand() % 5 + 1);
			printf("chef%d has delivered the dessert to the wholesaler\n", chef_id+1);
			if(sem_post(&wholesaler_ready) == -1) {
				perror("sem_post->wholesaler_ready");
				exit(EXIT_FAILURE);
			}
		}
		else 
			break;
	}
	return NULL;
}

void* pusher(void* arg)
{
	int id;
	id = *(int*) arg;
	while(true) {
		/* pusher will get active if the wholesaler comes and gets some ingredient */
		if(sem_wait(&mutex_pusher) == -1) {
			perror("sem_wait->mutex_pusher");
			exit(EXIT_FAILURE);
		}
		/* checks what is the ingredients what wholesaler got */
		/* if milk and flour, then increase it */
		/* if milk and walnuts, then increase it */
		/* if milk and sugar, then increase it */
		/* if flour and walnuts, then increase it */
		/* if flour and sugar, then increase it */
		/* if walnuts and sugar, then increase it */
		if((onechar[0] == 'M' && onechar[1] == 'F') || (onechar[0] == 'F' && onechar[1] == 'M')) {
			if(sem_post(&milk_flour) == -1) {
				perror("sem_post->milk_flour");
				exit(EXIT_FAILURE);
			}
		}
		else if((onechar[0] == 'M' && onechar[1] == 'W') || (onechar[0] == 'W' && onechar[1] == 'M')) {
			if(sem_post(&milk_walnuts) == -1) {
				perror("sem_post->milk_walnuts");
				exit(EXIT_FAILURE);
			}
		}
		else if((onechar[0] == 'M' && onechar[1] == 'S') || (onechar[0] == 'S' && onechar[1] == 'M')) {
			if(sem_post(&milk_sugar) == -1) {
				perror("sem_post->milk_sugar");
				exit(EXIT_FAILURE);
			}
		}
		else if((onechar[0] == 'F' && onechar[1] == 'W') || (onechar[0] == 'W' && onechar[1] == 'F')) {
			if(sem_post(&flour_walnuts) == -1) {
				perror("sem_post->flour_walnuts");
				exit(EXIT_FAILURE);
			}
		}
		else if((onechar[0] == 'F' && onechar[1] == 'S') || (onechar[0] == 'S' && onechar[1] == 'F')) {
			if(sem_post(&flour_sugar) == -1) {
				perror("sem_post->flour_sugar");
				exit(EXIT_FAILURE);
			}
		}
		else if((onechar[0] == 'W' && onechar[1] == 'S') || (onechar[0] == 'S' && onechar[1] == 'W')) {
			if(sem_post(&walnuts_sugar) == -1) {
				perror("sem_post->walnuts_sugar");
				exit(EXIT_FAILURE);
			}
		}
		else if(onechar[0]=='-' && id==0) {
			/* again post them if there is no more ingredients and chefs will be break and threads will be end   */
			if(sem_post(&milk_flour) == -1) {
				perror("sem_post->milk_flour");
				exit(EXIT_FAILURE);
			}
			if(sem_post(&milk_walnuts) == -1) {
				perror("sem_post->milk_walnuts");
				exit(EXIT_FAILURE);
			}
			if(sem_post(&milk_sugar) == -1) {
				perror("sem_post->milk_sugar");
				exit(EXIT_FAILURE);
			}
			if(sem_post(&flour_walnuts) == -1) {
				perror("sem_post->flour_walnuts");
				exit(EXIT_FAILURE);
			}
			if(sem_post(&flour_sugar) == -1) {
				perror("sem_post->flour_sugar");
				exit(EXIT_FAILURE);
			}
			if(sem_post(&walnuts_sugar) == -1) {
				perror("sem_post->walnuts_sugar");
				exit(EXIT_FAILURE);
			}
			break;
		}
		else 		
			break;
	}
	return NULL;
}

int main(int argc, char* argv[])
{
	int ss=0;
	/* keeps the name of the file */
	char* input_filename;
	int i;
	int opt, fd_in, read_success, read_;
	int pusher_ids[1], chef_ids[6];
	pthread_t pusher_threads[1], chef_threads[6];
	char empty[1];
	input_filename = (char*)malloc(30*sizeof(char));
	while((opt = getopt(argc, argv, "i")) != -1)	
	{	
		switch(opt)	
		{	
			case 'i':
				if(argv[optind]) {
					ss=1;
					strcpy(input_filename, argv[optind]);
				}
				break;
			default:
				errno = E2BIG;
				/* if some ekstra thing is given by commend line, print message with errno  */
				perror("The commond line should be: ./program -i filePath ");
				exit(EXIT_FAILURE);
				break;
		}	
	}
	/* if the parameters is missing, prints usage error  */
	if(ss == 0) {
		errno = ENOENT;
		perror("The file is not given \nThe commond line should be: ./program -i filePath ");
		exit(EXIT_FAILURE);
	}
	/* opens file  */
	fd_in = open(input_filename, O_RDONLY);
	if(fd_in == -1) {
		errno = EEXIST;
		perror("open");
		exit(EXIT_FAILURE);
	}

	/* initial values are assigned  */
	if(sem_init(&wholesaler_ready, 0, 1) == -1) {
		perror("sem_init->wholesaler_ready");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&mutex_pusher, 0, 0) == -1) {
		perror("sem_init->mutex_pusher");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&milk_flour, 0, 0) == -1) {
		perror("sem_init->milk_flour");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&milk_walnuts, 0, 0) == -1) {
		perror("sem_init->milk_walnuts");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&milk_sugar, 0, 0) == -1) {
		perror("sem_init->milk_sugar");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&flour_walnuts, 0, 0) == -1) {
		perror("sem_init->flour_walnuts");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&flour_sugar, 0, 0) == -1) {
		perror("sem_init->flour_sugar");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&walnuts_sugar, 0, 0) == -1) {
		perror("sem_init->walnuts_sugar");
		exit(EXIT_FAILURE);
	}

	/* setted pusher thread */
	pusher_ids[0]=0;
	if (pthread_create(&pusher_threads[0], NULL, pusher, &pusher_ids[0]) != 0) {
		perror("pthread_create->pusher_threads\nInsufficient resources to create thread");
		exit(EXIT_FAILURE);
	}

	/* setted chef threads */
	for(i=0; i<6; i++) {
		chef_ids[i] = i;
		if (pthread_create(&chef_threads[i], NULL, chef, &chef_ids[i]) != 0) {
			perror("pthread_create->chef_threads\nInsufficient resources to create thread");
			exit(EXIT_FAILURE);
		}
	}

	/* read ingredients from file one by one */
	/* if reaches EOF, close file and ingredients setted as - and pusher and chefs will understand, could be end  */
	onechar = (char*) malloc(2 * sizeof(char));
	while(true) {
		
		if(sem_wait(&wholesaler_ready) == -1) {
			perror("sem_wait->wholesaler_ready");
			exit(EXIT_FAILURE);
		}
			
		read_success = read(fd_in,onechar,2);
		read_ = read(fd_in,empty,1);

		if(read_success == 0 || read_ == 0) {
			if(close(fd_in) == -1) {
				perror("close");
				exit(EXIT_FAILURE);
			}
			onechar[0]='-';
			if(sem_post(&mutex_pusher) == -1) {
				perror("sem_post->mutex_pusher");
				exit(EXIT_FAILURE);
			}
		}
		if(read_success == 0 || read_ == 0) 
			break;

		if(read_success==-1 || read_==-1){
			errno = EINTR;
			perror("read");
			exit(EXIT_FAILURE);
		}

		if((onechar[0] == 'M' && onechar[1] == 'F') || (onechar[0] == 'F' && onechar[1] == 'M')) {
			printf("the wholesaler delivers milk and flour\n");
		}
		else if((onechar[0] == 'M' && onechar[1] == 'W') || (onechar[0] == 'W' && onechar[1] == 'M')) {
			printf("the wholesaler delivers milk and walnuts\n");
		}
		else if((onechar[0] == 'M' && onechar[1] == 'S') || (onechar[0] == 'S' && onechar[1] == 'M')) {
			printf("the wholesaler delivers milk and sugar\n");
		}
		else if((onechar[0] == 'F' && onechar[1] == 'W') || (onechar[0] == 'W' && onechar[1] == 'F')) {
			printf("the wholesaler delivers flour and walnuts\n");
		}
		else if((onechar[0] == 'F' && onechar[1] == 'S') || (onechar[0] == 'S' && onechar[1] == 'F')) {
			printf("the wholesaler delivers flour and sugar\n");
		}
		else if((onechar[0] == 'W' && onechar[1] == 'S') || (onechar[0] == 'S' && onechar[1] == 'W')) {
			printf("the wholesaler delivers walnuts and sugar\n");
		}
		printf("the wholesaler is waiting for the dessert\n");

		/* post pusher mutex and pusher can continue */
		if(sem_post(&mutex_pusher) == -1) {
			perror("sem_post->mutex_pusher");
			exit(EXIT_FAILURE);
		}
		/* wholesaaler waits until gullac comes */
		if(sem_wait(&wholesaler_ready) == -1) {
			perror("sem_wait->wholesaler_ready");
			exit(EXIT_FAILURE);
		}
		printf("the wholesaler has obtained the dessert and left to sell it\n");
		if(sem_post(&wholesaler_ready) == -1) {
			perror("sem_post->wholesaler_ready");
			exit(EXIT_FAILURE);
		}
	}

	/* waits until the other threads will be end */
	for (i = 0; i < 6; ++i) {
		if(pthread_join(chef_threads[i], NULL) != 0) {
			perror("pthread_join->chef_threads[i]");
			exit(EXIT_FAILURE);
		}
	}
	if(pthread_join(pusher_threads[0], NULL) != 0) {
		perror("pthread_join->pusher_threads[0]");
		exit(EXIT_FAILURE);
	}

	/* clean all the things */
	free(onechar);
	free(input_filename);
	if(sem_destroy(&mutex_pusher) == -1) {
		perror("sem_destroy->mutex_pusher");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&wholesaler_ready) == -1) {
		perror("sem_destroy->wholesaler_ready");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&milk_flour) == -1) {
		perror("sem_destroy->milk_flour");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&milk_walnuts) == -1) {
		perror("sem_destroy->milk_walnuts");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&milk_sugar) == -1) {
		perror("sem_destroy->milk_sugar");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&flour_walnuts) == -1) {
		perror("sem_destroy->flour_walnuts");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&flour_sugar) == -1) {
		perror("sem_destroy->flour_sugar");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&walnuts_sugar) == -1) {
		perror("sem_destroy->walnuts_sugar");
		exit(EXIT_FAILURE);
	}
	return 0;
}

