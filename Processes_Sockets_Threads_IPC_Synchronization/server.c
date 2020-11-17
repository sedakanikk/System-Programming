#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <netinet/in.h> 
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/socket.h> 
#include <sys/stat.h>
#include <sys/time.h> 
#include <sys/types.h>
#include <sys/wait.h> 
#include <time.h>
#include <unistd.h>

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX_INT 99999999
#define MIN_INT -99999999

typedef struct AdjacencyListNode { 
	int dest; 
	struct AdjacencyListNode* next; 
}AdjacencyListNode;

typedef struct AdjacencyList { 
	struct AdjacencyListNode *head;	
}AdjacencyList; 

typedef struct Graph {
	int V; 
	int *visited;
	struct AdjacencyList* array;
}Graph;

typedef struct queue {
	int front; 
	int rear;
	int *array;
}queue;

typedef struct cacheDS {
	int source;
	int destination;
	int size;
	int *path;
}cacheDS;

int check(int number);
void numbersNodeEdge(int filedes);
struct AdjacencyListNode* newAdjacencyNode(int dest);
void createGraph(int V);
void addEdge(int src, int dest);
int isEmpty(struct queue* q);
void modifyQueue(struct queue* q);
void addQueue(struct queue* q, int value);
int BFSandWriteDB(int start);
struct AdjacencyListNode* newNodeArr=NULL;
int nodeCount=0;

/* global variables */
int ind=0, flag = 0, fd_in=0, fd_out=0;
char *input_filename, *output_filename, *fd_write;
char *text_file=NULL;
char string_thread[10000];
int exit_signal = 1;

int *nodes, index_nodes=0, *array_from, *array_to, from_to=0;
int max_num=MIN_INT, min_num=MAX_INT;
int PORT=0, DESTINATION_NODE=0, SOURCE_NODE=0;
int startup_thread=0, maximum_thread=0;
struct Graph* graph = NULL ;
int socket_desc, sock=-1, clientLen;
int flag_dest=0;
pthread_cond_t cond_thread = PTHREAD_COND_INITIALIZER, cond_additional = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_waiting = PTHREAD_COND_INITIALIZER, cond_start = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_main = PTHREAD_MUTEX_INITIALIZER, mutex_additional = PTHREAD_MUTEX_INITIALIZER, mutex_m = PTHREAD_MUTEX_INITIALIZER;

int cache_index=0, signed_index=0;
struct cacheDS *cache;
struct sockaddr_in client, remote;

// threads  
int *ids;
pthread_t *threads;
int size_thread=1;
int usage_thread=0;
int priority = -1;

int AR = 0, AW = 0, WR = 0, WW = 0;
pthread_cond_t okToRead = PTHREAD_COND_INITIALIZER, okToWrite = PTHREAD_COND_INITIALIZER;

int read_res;
int visited_num = 0;


int count_nodes = 100;
int count_arrays = 100;

/* finds and prints date and time */
void timestamp(){
	time_t clock;
	char *date; 
	clock = time(NULL);
	date = ctime(&clock);
	date[strlen(date) - 1] = '\0'; 
	//printf("%s ",date);
	strcat(text_file, date);
	strcat(text_file, " ");
}

/* if the number is exists */
int check(int number) {
	int i;
	for(i=0; i<index_nodes; i++) {
		if(nodes[i] == number)
			return -1;
	}
	return 1;
}

/* calculates numbers of nodes and edges */
void numbersNodeEdge(int filedes) {
	char *buf, *temp;
	int i=0, j=0, num, flag=0;
	buf = (char*) malloc(120 * sizeof(char));
	temp = (char*) malloc(50 * sizeof(char));

	while(1) {

		if((read_res = read(filedes, temp, sizeof(char))) == -1) {
			perror("read->filedes: ");
			strcat(fd_write,"input file could not read!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		if(read_res == 0)
			break;
		if(*temp == '#') {
			while(*temp != '\n') {
				if((read_res = read(filedes, temp, sizeof(char))) == -1) {
					perror("read: ");
					strcat(fd_write,"input file could not read!\n");
					write(fd_out, fd_write, strlen(fd_write));
					exit(EXIT_FAILURE);
				}
				if(read_res == 0)
					break;
			}
		}
			
		while(*temp == '0' || *temp == '1' || *temp == '2' || *temp == '3' || *temp == '4' || *temp == '5' || *temp == '6' || *temp == '7' || *temp == '8' || *temp == '9') {
			buf[i] = *temp;
			i++;
			flag = 1;
			if((read_res = read(filedes, temp, sizeof(char))) == -1) {
				perror("read: ");
				strcat(fd_write,"input file could not read!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}
			if(read_res == 0)
				break;
		}
		if(flag == 1){
			buf[i] = '\0';
			sscanf(buf, "%d", &num);
			if(from_to == 0) {
				if(ind == count_arrays-2) {
					count_arrays *= 2;
					array_from = (int*)realloc(array_from, count_arrays*sizeof(int));
					array_to = (int*)realloc(array_to, count_arrays*sizeof(int));
				}
				array_from[ind] = num;
				from_to = 1;
			}
			else {
				if(ind == count_arrays-2) {
					count_arrays *= 2;
					array_from = (int*)realloc(array_from, count_arrays*sizeof(int));
					array_to = (int*)realloc(array_to, count_arrays*sizeof(int));
				}
				array_to[ind] = num;
				from_to = 0;
				ind++;
			}
			if (check(num) == 1) {
				if(index_nodes == count_nodes-2) {
					count_nodes *= 2;
					nodes = (int*)realloc(nodes, count_nodes*sizeof(int));
				}
				nodes[index_nodes] = num;
				if(num > max_num)
					max_num = num;
				if(num < min_num)
					min_num = num;
				index_nodes++;
			}
			for(j=0; j<=i; j++) 
				buf[j] = '\0';
			i=0;
			flag = 0;
		}
	}
	visited_num = index_nodes;
	free(temp);
	free(buf);
}

/* creates a graph which has number of vertices are V */
void createGraph(int V) { 
	int i, j; 
	graph = (struct Graph*)malloc(1*sizeof(struct Graph));
	graph->V = V; 
	graph->array = (struct AdjacencyList*) malloc(V * sizeof(struct AdjacencyList)); 
	graph->visited = (int*) malloc(index_nodes*index_nodes * sizeof(int));

	for (i = 0; i < V; ++i) {
		graph->array[i].head = (struct AdjacencyListNode*) malloc(V * sizeof(struct AdjacencyListNode)); 
		for(j=0; j<V; j++) {
			graph->array[i].head[j].next = NULL;
		}
	}
}

/* adds an edge into graph */
void addEdge(int src, int dest) {
	//struct AdjacencyListNode* newNode = newAdjacencyNode(dest);
	struct AdjacencyListNode* newNode = (struct AdjacencyListNode*) malloc(sizeof(struct AdjacencyListNode)); 
	newNode->dest = dest; 
	newNode->next = graph->array[src].head;
	graph->array[src].head = newNode;

}

/* check if the queue is empty or not */
int isEmpty(struct queue* q) {
	if (q->rear == -1)
		return 1;
	return 0;
}

/* checks if the given vertices are adjacent or not */
int isAdjacent(int src, int dest) {
	int i;
	for(i=0; i<ind; i++) {
		if(array_from[i] == src && array_to[i] == dest)
			return 1;
	}
	return 0;
}

void writeDatabase(int index, int arr[]) {
	int i;
	if(priority==0){
		if(pthread_mutex_lock(&mutex_m) != 0) {
			perror("pthread_mutex_lock->mutex_m:");
			strcat(fd_write,"The mutex mutex_m could not locked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		while ((WR + AW + AR) > 0) { // if any readers or writers, wait
			WW++; // waiting writer
			//cwait(okToWrite, m);
			if(pthread_cond_wait(&okToWrite, &mutex_m) != 0) {
				perror("pthread_cond_wait->okToWrite->mutex_m:");
				strcat(fd_write,"Condition okToWrite could not arrived!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}
			WW--;
		}
		AW++; // active writer
		if(pthread_mutex_unlock(&mutex_m) != 0) {
			perror("pthread_mutex_unlock->mutex_m:");
			strcat(fd_write,"The mutex mutex_m could not unlocked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}

		cache[cache_index].source = SOURCE_NODE;
		cache[cache_index].destination = DESTINATION_NODE;
		cache[cache_index].size = 0;
		if(index >= 1) {
			cache[cache_index].path = (int*)malloc(index_nodes*index_nodes*sizeof(int));
			signed_index = cache_index;
			for(i=0; i<index; i++) {
				cache[cache_index].path[i] = arr[i];
			}
			cache[cache_index].size = index;
		}
		++cache_index;

		if(pthread_mutex_lock(&mutex_m) != 0) {
			perror("pthread_mutex_lock->mutex_m:");
			strcat(fd_write,"The mutex mutex_m could not locked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		AW--;
		
		if (WR > 0) {
			if(pthread_cond_broadcast(&okToRead) != 0) {
				perror("pthread_cond_broadcast->okToRead:");
				strcat(fd_write,"Condition broadcast okToRead could not sent!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}
		}
		else if (WW > 0) {// give priority to other writers
			if(pthread_cond_signal(&okToWrite) != 0) {
				perror("pthread_cond_signal->okToWrite:");
				strcat(fd_write,"Condition okToWrite could not sent!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}
		}

		if(pthread_mutex_unlock(&mutex_m) != 0) {
			perror("pthread_mutex_unlock->mutex_m:");
			strcat(fd_write,"The mutex mutex_m could not unlocked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
	}
	else if( priority==-1 || priority==1){
		if(pthread_mutex_lock(&mutex_m) != 0) {
			perror("pthread_mutex_lock->mutex_m:");
			strcat(fd_write,"The mutex mutex_m could not locked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		while ((AW + AR) > 0) { // if any readers or writers, wait
			WW++; // waiting writer
			//cwait(okToWrite, m);
			if(pthread_cond_wait(&okToWrite, &mutex_m) != 0) {
				perror("pthread_cond_wait->okToWrite->mutex_m:");
				strcat(fd_write,"Condition okToWrite could not arrived!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}
			WW--;
		}
		AW++; // active writer
		if(pthread_mutex_unlock(&mutex_m) != 0) {
			perror("pthread_mutex_unlock->mutex_m:");
			strcat(fd_write,"The mutex mutex_m could not unlocked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}

		cache[cache_index].source = SOURCE_NODE;
		cache[cache_index].destination = DESTINATION_NODE;
		cache[cache_index].size = 0;
		if(index >= 1) {
			cache[cache_index].path = (int*)malloc(index_nodes*index_nodes*sizeof(int));
			signed_index = cache_index;
			for(i=0; i<index; i++) {
				cache[cache_index].path[i] = arr[i];
			}
			cache[cache_index].size = index;
		}
		++cache_index;

		if(pthread_mutex_lock(&mutex_m) != 0) {
			perror("pthread_mutex_lock->mutex_m:");
			strcat(fd_write,"The mutex mutex_m could not locked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		AW--;
		if (WW > 0) {// give priority to other writers
			if(pthread_cond_signal(&okToWrite) != 0) {
				perror("pthread_cond_signal->okToWrite:");
				strcat(fd_write,"Condition okToWrite could not sent!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}
		}
		else if (WR > 0) {
			if(pthread_cond_broadcast(&okToRead) != 0) {
				perror("pthread_cond_broadcast->okToRead:");
				strcat(fd_write,"Condition broadcast okToRead could not sent!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}
		}
		if(pthread_mutex_unlock(&mutex_m) != 0) {
			perror("pthread_mutex_unlock->mutex_m:");
			strcat(fd_write,"The mutex mutex_m could not unlocked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
	}
	else{
		if(pthread_mutex_lock(&mutex_m) != 0) {
			perror("pthread_mutex_unlock->mutex_m:");
			strcat(fd_write,"The mutex mutex_m could not unlocked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}

		cache[cache_index].source = SOURCE_NODE;
		cache[cache_index].destination = DESTINATION_NODE;
		cache[cache_index].size = 0;
		if(index >= 1) {
			cache[cache_index].path = (int*)malloc(index_nodes*index_nodes*sizeof(int));
			signed_index = cache_index;
			for(i=0; i<index; i++) {
				cache[cache_index].path[i] = arr[i];
			}
			cache[cache_index].size = index;
		}
		++cache_index;

		if(pthread_mutex_unlock(&mutex_m) != 0) {
			perror("pthread_mutex_lock->mutex_m:");
			strcat(fd_write,"The mutex mutex_m could not locked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}		
	}
}

/* modifies the queue after BFSandWriteDB, checks from rear the queue to front looks for adjacency between two vertices */
void modifyQueue(struct queue* q) {
	int i, j;
	int arr[ind];
	int index=0, new_index;
	q->front = 0;
	
	for (i = q->front; i < q->rear + 1; i++, index++) 
		arr[index] = q->array[i];
	new_index = index;	
	i=index-2;
	while(i>=0) {

		if(isAdjacent(arr[i], arr[i+1]) != 1) {
			new_index--;
			j=i;
			while(j<index) {
				arr[j] = arr[j+1];
				j++;
			}
		}
		i--;
	}
	writeDatabase(new_index, arr);
}

/* adding elements into queue */
void addQueue(struct queue* q, int value) {
	if(flag_dest == 0) {
		if (q->front == -1)
			q->front = 0;
		q->rear++;
		q->array[q->rear] = value;
	}
	if(value == DESTINATION_NODE) 
		flag_dest = 1;
	
}

/* checks if the vertex is source */
int isSource(int src) {
	int i;
	for(i=0; i<ind; i++) {
		if(array_from[i] == src)
			return 1;
	}
	return 0;
}

/* checks if the vertex is destination */
int isDestination(int dest) {
	int i;
	for(i=0; i<ind; i++) {
		if(array_to[i] == dest)
			return 1;
	}
	return 0;
}

// Removing elements from queue
int removeQueue(struct queue* q) {
  int item;
	if (isEmpty(q)) {
		//printf("Queue is empty");
		item = -1;
	}
	else {
		item = q->array[q->front];
		q->front++;
		if (q->front > q->rear) {
			//printf("Resetting queue ");
			q->front = q->rear = -1;
		}
	}
  return item;
}


int flag_ret=0;
/* visites vertices with BFSandWriteDB and adds queue, then modifies it */
int BFSandWriteDB(int start) {
	struct queue* q = (struct queue*) malloc((max_num-min_num+1) * sizeof(struct queue));
	q->array = (int*) malloc((max_num-min_num+1) * sizeof(int));
	q->front = -1;
	q->rear = -1;

	struct queue* last_queue = (struct queue*) malloc((max_num-min_num+1) * sizeof(struct queue));
	last_queue->array = (int*) malloc((max_num-min_num+1) * sizeof(int));
	last_queue->front = -1;
	last_queue->rear = -1;

	if(isSource(start) == 0 || isDestination(DESTINATION_NODE) == 0) {
		free(q->array);
		free(q);
		free(last_queue->array);
		free(last_queue);
		return 0;
	}
	
	if(SOURCE_NODE == DESTINATION_NODE && isAdjacent(SOURCE_NODE, DESTINATION_NODE)==1) {
		cache[cache_index].source = SOURCE_NODE;
		cache[cache_index].destination = DESTINATION_NODE;
		cache[cache_index].size = 2;
	
		cache[cache_index].path = (int*)malloc(2*sizeof(int));
		signed_index = cache_index;
		cache[cache_index].path[0] = SOURCE_NODE;
		cache[cache_index].path[1] = DESTINATION_NODE;
		++cache_index;
		free(q->array);
		free(q);
		free(last_queue->array);
		free(last_queue);

		return 1;
	}
	
	graph->visited[start] = 1;
	addQueue(q, start);
	addQueue(last_queue, start);
	//printf("adjVertex: %d\n", start);

	while (!isEmpty(q)) {
		int currentVertex = removeQueue(q);
		struct AdjacencyListNode* temp = graph->array[currentVertex].head;

		while (temp!=NULL && temp->next!=NULL) {
			//printf("HERE\n");
			int adjVertex = temp->dest;
			//printf("adjVertex: %d\n", adjVertex);
			//printf("index_nodes: %d\n", index_nodes);


			if (graph->visited[adjVertex] == 0) {
				graph->visited[adjVertex] = 1;
				addQueue(last_queue, adjVertex);
				addQueue(q, adjVertex);
			}
			temp = temp->next;
		}
		if(flag_dest == 1) {
			modifyQueue(last_queue);
			flag_dest = 0;
			break; 
		}
		free(temp);
	}
	free(q->array);
	free(q);
	free(last_queue->array);
	free(last_queue);
	return 1;
}

/* searches database for a taken path from client */
int searchDatabase(int src, int dest) {
	int i;
	if(cache_index == 0)
		return 0;
	for(i=0; i<cache_index; i++) {
		if(cache[i].source == src && cache[i].destination == dest) {
			signed_index = i;
			return 1;
		}
	}
	return 0;
}

/* initalizes cache data structure */
void initialize_cache() {
	cache = (cacheDS*)malloc(index_nodes*index_nodes*sizeof(cacheDS));
	cache_index=0;
}


/* server thread function */
void *pool (void* arg) {
	int thread_id = *(int*) arg;

	int arr[2], i;
	int sock_local=-1;
	
	//Accept and incoming connection
	while(1) {
		if(exit_signal == 0){
			//printf("breakkkkkkkk1 th=%d\n", thread_id);
			if(pthread_cond_signal(&cond_thread) != 0) {
				perror("pthread_cond_signal->cond_start:");
				strcat(fd_write,"Condition cond_start could not sent!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}	

			if(pthread_mutex_unlock(&mutex_main) != 0) {
				perror("pthread_mutex_lock->mutex_main:");
				strcat(fd_write,"The mutex mutex_main could not locked!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}					
			break;
		}

		timestamp();
		//printf("Thread #%d: waiting for connection...\n", thread_id);
		strcat(text_file, "Thread #");
		sprintf(string_thread, "%d", thread_id);
		strcat(text_file, string_thread);
		strcat(text_file, ": waiting for connection...\n");
		write(fd_out, text_file, strlen(text_file));
		//printf("text_file: %s\n", text_file);
		memset(text_file, '\0', strlen(text_file));
	

		if(pthread_mutex_lock(&mutex_main) != 0) {
			perror("pthread_mutex_lock->mutex_main:");
			strcat(fd_write,"The mutex mutex_main could not locked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}

		if(exit_signal == 0){
			if(pthread_cond_signal(&cond_thread) != 0) {
				perror("pthread_cond_signal->cond_start:");
				strcat(fd_write,"Condition cond_start could not sent!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}

			if(pthread_cond_signal(&cond_additional) != 0) {
				perror("pthread_cond_signal->cond_start:");
				strcat(fd_write,"Condition cond_start could not sent!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}				

			if(pthread_mutex_unlock(&mutex_main) != 0) {
				perror("pthread_mutex_lock->mutex_main:");
				strcat(fd_write,"The mutex mutex_main could not locked!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}					
			break;			
		}

		while(sock == -1){
			//printf("%d locked mutex_main and waits for cond_thread\n", thread_id);
			if(pthread_cond_wait(&cond_thread, &mutex_main) != 0) {
				perror("pthread_cond_wait->cond_thread->mutex_main:");
				strcat(fd_write,"Condition cond_thread could not arrived!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}
			if(exit_signal == 0 && sock == -1){
				//printf("breakkkkkkkk2 th=%d\n", thread_id);
				break;
			}
		}

		if(exit_signal == 0 && sock == -1){
			//printf("breakkkkkkkk222 th=%d\n", thread_id);
			if(pthread_cond_signal(&cond_thread) != 0) {
				perror("pthread_cond_wait->cond_thread->mutex_main:");
				strcat(fd_write,"Condition cond_thread could not arrived!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}	
			if(pthread_mutex_unlock(&mutex_main) != 0) {
				perror("pthread_mutex_lock->mutex_main:");
				strcat(fd_write,"The mutex mutex_main could not locked!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}					
			break;
		}

		sock_local = sock ;
		sock = -1;

		if(pthread_cond_signal(&cond_start) != 0) {
			perror("pthread_cond_signal->cond_start:");
			strcat(fd_write,"Condition cond_start could not sent!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}

		if(pthread_mutex_unlock(&mutex_main) != 0) {
			perror("pthread_mutex_unlock->mutex_main");
			strcat(fd_write,"The mutex mutex_main could not unlocked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}

		//sleep(10);

		//printf("sleepsleepsleepsleepsleepsleepsleepsleep\n");
		if(pthread_mutex_lock(&mutex_additional) != 0) {
			perror("pthread_mutex_lock->mutex_additional:");
			strcat(fd_write,"The mutex mutex_additional could not locked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}

		//printf("%d locked mutex_additional\n", thread_id);
		++usage_thread;
		float us = (usage_thread*1.0) / (size_thread*1.0)*100;
		int us_int = us;
		timestamp();
		//printf("A connection has been delegated to thread id #%d, system load %.0f%%\n", thread_id, us);
		sprintf(string_thread, "%d", thread_id);
		strcat(text_file, "A connection has been delegated to thread id #");
		strcat(text_file, string_thread);
		strcat(text_file, ", system load ");
		sprintf(string_thread, "%d", us_int);
		strcat(text_file, string_thread);
		strcat(text_file, "%\n");
		write(fd_out, text_file, strlen(text_file));
		//printf("text_file: %s\n", text_file);
		memset(text_file, '\0', strlen(text_file));
	
		//printf("%d signal cond_waiting sent\n", thread_id);
		if(pthread_cond_signal(&cond_additional) != 0) {
			perror("pthread_cond_signal->cond_additional:");
			strcat(fd_write,"Condition cond_additional could not sent!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		if(pthread_cond_signal(&cond_waiting) != 0) {
			perror("pthread_cond_signal->cond_waiting:");
			strcat(fd_write,"Condition cond_waiting could not sent!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		//printf("%d signal cond_additional sent\n", thread_id);

		if(pthread_mutex_unlock(&mutex_additional) != 0) {
			perror("pthread_mutex_unlock->mutex_additional:");
			strcat(fd_write,"The mutex mutex_additional could not unlocked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		//printf("%d unlocked mutex_additional\n", thread_id);
	
		//sleep(10);
	
		if((read_res = read(sock_local, arr, sizeof(arr))) == -1) {
			perror("read: ");
			strcat(fd_write,"client could not read!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		if(read_res == 0)
			break;

		SOURCE_NODE = arr[0];
		DESTINATION_NODE = arr[1];

		timestamp();
		//printf("Thread #%d: searching database for a path from node %d to node %d\n", thread_id, arr[0], arr[1]);
		strcat(text_file, "Thread #");
		sprintf(string_thread, "%d", thread_id);
		strcat(text_file, string_thread);
		strcat(text_file, ": searching database for a path from node ");
		sprintf(string_thread, "%d", arr[0]);
		strcat(text_file, string_thread);
		strcat(text_file, " to node ");
		sprintf(string_thread, "%d", arr[1]);
		strcat(text_file, string_thread);
		strcat(text_file, "\n");
		write(fd_out, text_file, strlen(text_file));
		//printf("text_file: %s\n", text_file);
		memset(text_file, '\0', strlen(text_file));



		if(priority==0){
			if(pthread_mutex_lock(&mutex_m) != 0) {
				perror("pthread_mutex_lock->mutex_m:");
				strcat(fd_write,"The mutex mutex_m could not locked!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}
			while (AW > 0) { // if any writers, wait
				WR++; // waiting reader	
				if(pthread_cond_wait(&okToRead, &mutex_m) != 0) {
					perror("pthread_cond_wait->okToRead->mutex_m:");
					strcat(fd_write,"Condition okToRead could not arrived!\n");
					write(fd_out, fd_write, strlen(fd_write));
					exit(EXIT_FAILURE);
				}
				WR--;
			}
			AR++; // active reader
			if(pthread_mutex_unlock(&mutex_m) != 0) {
				perror("pthread_mutex_unlock->mutex_m:");
				strcat(fd_write,"The mutex mutex_m could not unlocked!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}

			int temp = searchDatabase(arr[0], arr[1]);

			if(pthread_mutex_lock(&mutex_m) != 0) {
				perror("pthread_mutex_lock->mutex_m:");
				strcat(fd_write,"The mutex mutex_m could not locked!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}
			AR--;
			if (AR == 0 && WR ==0 && WW>0) {
				if(pthread_cond_signal(&okToWrite) != 0) {
					perror("pthread_cond_signal->okToWrite:");
					strcat(fd_write,"Condition okToWrite could not sent!\n");
					write(fd_out, fd_write, strlen(fd_write));
					exit(EXIT_FAILURE);
				}
			}
			if(pthread_mutex_unlock(&mutex_m) != 0) {
				perror("pthread_mutex_unlock->mutex_m:");
				strcat(fd_write,"The mutex mutex_m could not unlocked!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}		

			if(temp == 0) {
				if(BFSandWriteDB(arr[0]) == 1) {
					timestamp();
					//printf("Thread #%d: no path in database, calculating %d->%d\n", thread_id, arr[0], arr[1]);
					strcat(text_file, "Thread #");
					sprintf(string_thread, "%d", thread_id);
					strcat(text_file, string_thread);
					strcat(text_file, ": no path in database, calculating ");
					sprintf(string_thread, "%d", arr[0]);
					strcat(text_file, string_thread);
					strcat(text_file, "->");
					sprintf(string_thread, "%d", arr[1]);
					strcat(text_file, string_thread);
					strcat(text_file, "\n");
					write(fd_out, text_file, strlen(text_file));
					//printf("text_file: %s\n", text_file);
					memset(text_file, '\0', strlen(text_file));
				

					write(sock_local, &(cache[signed_index].size), sizeof(cache[signed_index].size));

					timestamp();
					//printf("Thread #%d: path calculated: ", thread_id);
					sprintf(string_thread, "%d", thread_id);
					strcat(text_file, "Thread #");
					strcat(text_file, string_thread);
					strcat(text_file, ": path calculated: ");

					for(i=0; i<cache[signed_index].size; i++) {
						write(sock_local, &(cache[signed_index].path[i]), sizeof(cache[signed_index].path[i]));

						//printf("%d", cache[signed_index].path[i]);
						sprintf(string_thread, "%d", cache[signed_index].path[i]);
						strcat(text_file, string_thread);						

						if(i != cache[signed_index].size-1) {
							//printf("->");
							strcat(text_file, "->");
						}
					}
					//printf("\n");
					strcat(text_file, "\n");
					write(fd_out, text_file, strlen(text_file));
					//printf("text_file: %s\n", text_file);
					memset(text_file, '\0', strlen(text_file));
				
				}
				else {
					timestamp();
					//printf("Thread #%d: path not possible from node %d to %d\n", thread_id, arr[0], arr[1]);
					sprintf(string_thread, "%d", thread_id);
					strcat(text_file, "Thread #");
					strcat(text_file, string_thread);
					strcat(text_file, ": path not possible from node ");
					sprintf(string_thread, "%d", arr[0]);
					strcat(text_file, string_thread);
					strcat(text_file, " to ");
					sprintf(string_thread, "%d", arr[1]);
					strcat(text_file, string_thread);
					strcat(text_file, "\n");
					write(fd_out, text_file, strlen(text_file));
					//printf("text_file: %s\n", text_file);
					memset(text_file, '\0', strlen(text_file));
				

					int zero=0;
					write(sock_local, &zero, sizeof(zero));
				}
				timestamp();
				//printf("Thread #%d: responding to client and adding path to database\n", thread_id);
				strcat(text_file, "Thread #");
				sprintf(string_thread, "%d", thread_id);
				strcat(text_file, string_thread);
				strcat(text_file, ": responding to client and adding path to database\n");
				write(fd_out, text_file, strlen(text_file));
				//printf("text_file: %s\n", text_file);
				memset(text_file, '\0', strlen(text_file));
			
				arr[0] = 0;
				arr[1] = 0;
				int i;
				for(i=0; i<index_nodes*index_nodes; i++) {
					graph->visited[i] = 0;	
				}
			}
			else {
				timestamp();
				//printf("Thread #%d: path found in database: ", thread_id);
				strcat(text_file, "Thread #");
				sprintf(string_thread, "%d", thread_id);
				strcat(text_file, string_thread);
				strcat(text_file, ": path found in database: ");


				write(sock_local, &(cache[signed_index].size), sizeof(cache[signed_index].size));
				for(i=0; i<cache[signed_index].size; i++) {
					write(sock_local, &(cache[signed_index].path[i]), sizeof(cache[signed_index].path[i]));

					//printf("%d", cache[signed_index].path[i]);
					sprintf(string_thread, "%d", cache[signed_index].path[i]);
					strcat(text_file, string_thread);

					if(i != cache[signed_index].size-1) {
						//printf("->");
						strcat(text_file, "->");
					}
				}
				//printf("\n");
				strcat(text_file, "\n");
				write(fd_out, text_file, strlen(text_file));
				//printf("text_file: %s\n", text_file);
				memset(text_file, '\0', strlen(text_file));
			
			}
		}

		/********************************************/
		else if(priority==-1 || priority==1){
	

			if(pthread_mutex_lock(&mutex_m) != 0) {
				perror("pthread_mutex_lock->mutex_m:");
				strcat(fd_write,"The mutex mutex_m could not locked!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}
			while ((AW + WW) > 0) { // if any writers, wait
				WR++; // waiting reader	
				if(pthread_cond_wait(&okToRead, &mutex_m) != 0) {
					perror("pthread_cond_wait->okToRead->mutex_m:");
					strcat(fd_write,"Condition okToRead could not arrived!\n");
					write(fd_out, fd_write, strlen(fd_write));
					exit(EXIT_FAILURE);
				}
				WR--;
			}
			AR++; // active reader
			if(pthread_mutex_unlock(&mutex_m) != 0) {
				perror("pthread_mutex_unlock->mutex_m:");
				strcat(fd_write,"The mutex mutex_m could not unlocked!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}

			int temp = searchDatabase(arr[0], arr[1]);

			if(pthread_mutex_lock(&mutex_m) != 0) {
				perror("pthread_mutex_lock->mutex_m:");
				strcat(fd_write,"The mutex mutex_m could not locked!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}
			AR--;
			if (AR == 0 && WW > 0) {
				if(pthread_cond_signal(&okToWrite) != 0) {
					perror("pthread_cond_signal->okToWrite:");
					strcat(fd_write,"Condition okToWrite could not sent!\n");
					write(fd_out, fd_write, strlen(fd_write));
					exit(EXIT_FAILURE);
				}
			}
			if(pthread_mutex_unlock(&mutex_m) != 0) {
				perror("pthread_mutex_unlock->mutex_m:");
				strcat(fd_write,"The mutex mutex_m could not unlocked!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}		

			if(temp == 0) {
				if(BFSandWriteDB(arr[0]) == 1) {
					timestamp();
					//printf("Thread #%d: no path in database, calculating %d->%d\n", thread_id, arr[0], arr[1]);
					strcat(text_file, "Thread #");
					sprintf(string_thread, "%d", thread_id);
					strcat(text_file, string_thread);
					strcat(text_file, ": no path in database, calculating ");
					sprintf(string_thread, "%d", arr[0]);
					strcat(text_file, string_thread);
					strcat(text_file, "->");
					sprintf(string_thread, "%d", arr[1]);
					strcat(text_file, string_thread);
					strcat(text_file, "\n");
					write(fd_out, text_file, strlen(text_file));
					//printf("text_file: %s\n", text_file);
					memset(text_file, '\0', strlen(text_file));
				

					write(sock_local, &(cache[signed_index].size), sizeof(cache[signed_index].size));

					timestamp();
					//printf("Thread #%d: path calculated: ", thread_id);
					sprintf(string_thread, "%d", thread_id);
					strcat(text_file, "Thread #");
					strcat(text_file, string_thread);
					strcat(text_file, ": path calculated: ");

					for(i=0; i<cache[signed_index].size; i++) {
						write(sock_local, &(cache[signed_index].path[i]), sizeof(cache[signed_index].path[i]));

						//printf("%d", cache[signed_index].path[i]);
						sprintf(string_thread, "%d", cache[signed_index].path[i]);
						strcat(text_file, string_thread);						

						if(i != cache[signed_index].size-1) {
							//printf("->");
							strcat(text_file, "->");
						}
					}
					//printf("\n");
					strcat(text_file, "\n");
					write(fd_out, text_file, strlen(text_file));
					//printf("text_file: %s\n", text_file);
					memset(text_file, '\0', strlen(text_file));
				
				}
				else {
					timestamp();
					//printf("Thread #%d: path not possible from node %d to %d\n", thread_id, arr[0], arr[1]);
					sprintf(string_thread, "%d", thread_id);
					strcat(text_file, "Thread #");
					strcat(text_file, string_thread);
					strcat(text_file, ": path not possible from node ");
					sprintf(string_thread, "%d", arr[0]);
					strcat(text_file, string_thread);
					strcat(text_file, " to ");
					sprintf(string_thread, "%d", arr[1]);
					strcat(text_file, string_thread);
					strcat(text_file, "\n");
					write(fd_out, text_file, strlen(text_file));
					//printf("text_file: %s\n", text_file);
					memset(text_file, '\0', strlen(text_file));
				

					int zero=0;
					write(sock_local, &zero, sizeof(zero));
				}
				timestamp();
				//printf("Thread #%d: responding to client and adding path to database\n", thread_id);
				strcat(text_file, "Thread #");
				sprintf(string_thread, "%d", thread_id);
				strcat(text_file, string_thread);
				strcat(text_file, ": responding to client and adding path to database\n");
				write(fd_out, text_file, strlen(text_file));
				//printf("text_file: %s\n", text_file);
				memset(text_file, '\0', strlen(text_file));
			
				arr[0] = 0;
				arr[1] = 0;
				int i;
				for(i=0; i<index_nodes*index_nodes; i++) {
					graph->visited[i] = 0;	
				}
			}
			else {
				timestamp();
				//printf("Thread #%d: path found in database: ", thread_id);
				strcat(text_file, "Thread #");
				sprintf(string_thread, "%d", thread_id);
				strcat(text_file, string_thread);
				strcat(text_file, ": path found in database: ");


				write(sock_local, &(cache[signed_index].size), sizeof(cache[signed_index].size));
				for(i=0; i<cache[signed_index].size; i++) {
					write(sock_local, &(cache[signed_index].path[i]), sizeof(cache[signed_index].path[i]));

					//printf("%d", cache[signed_index].path[i]);
					sprintf(string_thread, "%d", cache[signed_index].path[i]);
					strcat(text_file, string_thread);

					if(i != cache[signed_index].size-1) {
						//printf("->");
						strcat(text_file, "->");
					}
				}
				//printf("\n");
				strcat(text_file, "\n");
				write(fd_out, text_file, strlen(text_file));
				//printf("text_file: %s\n", text_file);
				memset(text_file, '\0', strlen(text_file));
			
			}
		}


		/****************************************/

		else{
			if(pthread_mutex_lock(&mutex_m) != 0) {
				perror("pthread_mutex_lock->mutex_m:");
				strcat(fd_write,"The mutex mutex_m could not locked!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}

			int temp = searchDatabase(arr[0], arr[1]);

			if(pthread_mutex_unlock(&mutex_m) != 0) {
				perror("pthread_mutex_lock->mutex_m:");
				strcat(fd_write,"The mutex mutex_m could not locked!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}

			if(temp == 0) {
				if(BFSandWriteDB(arr[0]) == 1) {
					timestamp();
					//printf("Thread #%d: no path in database, calculating %d->%d\n", thread_id, arr[0], arr[1]);
					strcat(text_file, "Thread #");
					sprintf(string_thread, "%d", thread_id);
					strcat(text_file, string_thread);
					strcat(text_file, ": no path in database, calculating ");
					sprintf(string_thread, "%d", arr[0]);
					strcat(text_file, string_thread);
					strcat(text_file, "->");
					sprintf(string_thread, "%d", arr[1]);
					strcat(text_file, string_thread);
					strcat(text_file, "\n");
					write(fd_out, text_file, strlen(text_file));
					//printf("text_file: %s\n", text_file);
					memset(text_file, '\0', strlen(text_file));
				

					write(sock_local, &(cache[signed_index].size), sizeof(cache[signed_index].size));

					timestamp();
					//printf("Thread #%d: path calculated: ", thread_id);
					sprintf(string_thread, "%d", thread_id);
					strcat(text_file, "Thread #");
					strcat(text_file, string_thread);
					strcat(text_file, ": path calculated: ");

					for(i=0; i<cache[signed_index].size; i++) {
						write(sock_local, &(cache[signed_index].path[i]), sizeof(cache[signed_index].path[i]));

						//printf("%d", cache[signed_index].path[i]);
						sprintf(string_thread, "%d", cache[signed_index].path[i]);
						strcat(text_file, string_thread);						

						if(i != cache[signed_index].size-1) {
							//printf("->");
							strcat(text_file, "->");
						}
					}
					//printf("\n");
					strcat(text_file, "\n");
					write(fd_out, text_file, strlen(text_file));
					//printf("text_file: %s\n", text_file);
					memset(text_file, '\0', strlen(text_file));
				
				}
				else {
					timestamp();
					//printf("Thread #%d: path not possible from node %d to %d\n", thread_id, arr[0], arr[1]);
					sprintf(string_thread, "%d", thread_id);
					strcat(text_file, "Thread #");
					strcat(text_file, string_thread);
					strcat(text_file, ": path not possible from node ");
					sprintf(string_thread, "%d", arr[0]);
					strcat(text_file, string_thread);
					strcat(text_file, " to ");
					sprintf(string_thread, "%d", arr[1]);
					strcat(text_file, string_thread);
					strcat(text_file, "\n");
					write(fd_out, text_file, strlen(text_file));
					//printf("text_file: %s\n", text_file);
					memset(text_file, '\0', strlen(text_file));
				

					int zero=0;
					write(sock_local, &zero, sizeof(zero));
				}
				timestamp();
				//printf("Thread #%d: responding to client and adding path to database\n", thread_id);
				strcat(text_file, "Thread #");
				sprintf(string_thread, "%d", thread_id);
				strcat(text_file, string_thread);
				strcat(text_file, ": responding to client and adding path to database\n");
				write(fd_out, text_file, strlen(text_file));
				//printf("text_file: %s\n", text_file);
				memset(text_file, '\0', strlen(text_file));
			
				arr[0] = 0;
				arr[1] = 0;
				int i;
				for(i=0; i<index_nodes*index_nodes; i++) {
					graph->visited[i] = 0;	
				}
			}
			else {
				timestamp();
				//printf("Thread #%d: path found in database: ", thread_id);
				strcat(text_file, "Thread #");
				sprintf(string_thread, "%d", thread_id);
				strcat(text_file, string_thread);
				strcat(text_file, ": path found in database: ");


				write(sock_local, &(cache[signed_index].size), sizeof(cache[signed_index].size));
				for(i=0; i<cache[signed_index].size; i++) {
					write(sock_local, &(cache[signed_index].path[i]), sizeof(cache[signed_index].path[i]));

					//printf("%d", cache[signed_index].path[i]);
					sprintf(string_thread, "%d", cache[signed_index].path[i]);
					strcat(text_file, string_thread);

					if(i != cache[signed_index].size-1) {
						//printf("->");
						strcat(text_file, "->");
					}
				}
				//printf("\n");
				strcat(text_file, "\n");
				write(fd_out, text_file, strlen(text_file));
				//printf("text_file: %s\n", text_file);
				memset(text_file, '\0', strlen(text_file));
			
			}
		}		
		
		close(sock_local);
		//printf("mmmmmmmmmmmmmmmmmmm\n");
		if(pthread_mutex_lock(&mutex_additional) != 0) {
			perror("pthread_mutex_lock->mutex_additional");
			strcat(fd_write,"The mutex mutex_additional could not locked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		//printf("%d locked mutex_additional\n", thread_id);
		//printf("usage_thread: %d\n", usage_thread);
		--usage_thread;
		//printf("usage_thread: %d\n", usage_thread);
		if(pthread_cond_signal(&cond_waiting) != 0) {
			perror("pthread_cond_signal->cond_waiting:");
			strcat(fd_write,"Condition cond_waiting could not sent!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		/*
		//printf("%d signal cond_waiting sent\n", thread_id);
		if(pthread_cond_signal(&cond_additional) != 0) {
			perror("pthread_cond_signal->cond_additional:");
			strcat(fd_write,"Condition cond_additional could not sent!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		*/
		//printf("%d sent cond_waiting\n", thread_id);
		if(pthread_mutex_unlock(&mutex_additional) != 0) {
			perror("pthread_mutex_unlock->mutex_additional");
			strcat(fd_write,"The mutex mutex_additional could not unlocked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		//printf("%d nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn\n", thread_id);
	}
	return NULL;
}

/* additional thread function */
void *additional (/*void* arg*/) {
	//int thread_id = *(int*) arg;
float comp_;
	
	while(1) {
		if(pthread_mutex_lock(&mutex_additional) != 0) {
			perror("pthread_mutex_lock->mutex_additional:");
			strcat(fd_write,"The mutex mutex_additional could not locked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		float comp = ((usage_thread*1.0)/(size_thread*1.0))*100;
		float numf = (size_thread*1.0)*0.25;
		int num = numf;
		while(comp < 75.0 || num==0) {
			//printf("comp = %f\n", comp);
			//printf("num = %d\n", num);
			//printf("indide jbhkl\n");
			if(pthread_cond_wait(&cond_additional, &mutex_additional) != 0) {
				perror("pthread_cond_wait->cond_additional->mutex_additional:");
				strcat(fd_write,"Condition cond_additional could not arrived!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}
			//printf("çıktııııııı\n" );
			comp = (usage_thread*1.0) / (size_thread*1.0)*100;
			comp_ = comp;
			if(exit_signal == 0){
				//printf("breakkkk additional\n");
				if(pthread_mutex_unlock(&mutex_additional) != 0) {
					perror("pthread_mutex_lock->mutex_additional:");
					strcat(fd_write,"The mutex mutex_additional could not locked!\n");
					write(fd_out, fd_write, strlen(fd_write));
					exit(EXIT_FAILURE);
				}	

				break;		
			}
		}
		if( exit_signal == 0)
			break;

		size_thread += num;

		strcat(text_file, "System load ");
		sprintf(string_thread, "%.1f", comp_);
		strcat(text_file, string_thread);
		strcat(text_file, "%, pool extended to 10 threads");
		strcat(text_file, "\n");
		write(fd_out, text_file, strlen(text_file));
		memset(text_file, '\0', strlen(text_file));	

		if(pthread_mutex_unlock(&mutex_additional) != 0) {
			perror("pthread_mutex_unlock->mutex_additional");
			strcat(fd_write,"The mutex mutex_additional could not unlocked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		//printf("cond geçti\n");
		/*
		if(thread_id!=-1) {
			//printf("usage_thread: %d\n", usage_thread);
			//printf("comp: %f\n", comp);
			//printf("uretilecek threads: %d\n", num);
		}
		*/

		if(size_thread+num > maximum_thread)
			num = maximum_thread-size_thread;			

		// threads created 
		for(int i=size_thread; i<size_thread+num; i++) {
			ids[i] = i;
			if (pthread_create(&threads[i], NULL, pool, &ids[i]) != 0) {
				perror("pthread_create->threads\nInsufficient resources to create thread");
				strcat(fd_write,"Thread could not created!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}
		}
	}

	for (int i = startup_thread; i < size_thread; ++i) {
		if(pthread_join(threads[i], NULL) == -1) {
			perror("pthread_join->threads:");
			strcat(fd_write,"Thread could not killed!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
	}
	return NULL;
}


void handler(int sig) {
  if(sig == SIGINT) {
  	//printf("handler\n");
    exit_signal = 0;

	strcat(text_file, "Termination signal received, waiting for ongoing threads to complete.\n");
	write(fd_out, text_file, strlen(text_file));
	//printf("text_file: %s\n", text_file);
	memset(text_file, '\0', strlen(text_file));	 

	if(pthread_cond_signal(&cond_additional) != 0) {
		perror("pthread_cond_signal->cond_waiting:");
		strcat(fd_write,"Condition cond_waiting could not sent!\n");
		write(fd_out, fd_write, strlen(fd_write));
		exit(EXIT_FAILURE);
	}    
  }
}

void becomeDaemon(){
    /* Our process ID and Session ID */
    pid_t pid, sid;
        
    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
	
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
       we can exit the parent process. */
    if (pid > 0) {

    	free(input_filename);
    	free(output_filename);
    	free(fd_write);
    	free(text_file);
    	free(nodes);
    	free(array_from);
    	free(array_to);

		for (int i = 0; i < max_num-min_num+1; ++i) {
			
			//printf("i=%d\n", i);
			for(int j=0; j<max_num-min_num+1; j++) {
				//printf("graph->array[i].head[j].next: %s\n", graph->array[i].head[j].next );
				//printf("j=%d     ", j);
				//if(graph->array[i].head[j].next != NULL)
			}
			///printf("---\n");
			
			free(graph->array[i].head);
					free(graph->array[i].head->next);
		}
		
		free(graph->array);
		free(graph->visited);
		free(graph);

		exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(0);
            
    /* Open any logs here */        
            
    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
            /* Log the failure */
            exit(EXIT_FAILURE);
    }
    
    /* Change the current working directory */
    if ((chdir("/")) < 0) {
            /* Log the failure */
            exit(EXIT_FAILURE);
    }
    
    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}


/* main thread */
int main(int argc, char *argv[]) {
	int ss=0, i, opt;
	struct sigaction sa;
	//sigset_t blockMask;
	srand(time(NULL));

	input_filename = (char*)malloc(30*sizeof(char));
	output_filename = (char*)malloc(30*sizeof(char));
	fd_write = (char*)malloc(300*sizeof(char));
	
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
	/*if(sigemptyset(&blockMask) == -1) {
		perror("sigemptyset:");
		exit(EXIT_FAILURE);
	}
	if(sigaddset(&blockMask, SIGINT) == -1) {
		perror("sigaddset:\n");
		exit(EXIT_FAILURE);
	}*/
	
	int flag_r = 0;

	while((opt = getopt(argc, argv, "iposxr")) != -1) {	
		switch(opt)	{	
			case 'i':
				if(argv[optind]) {
					ss++;
					strcpy(input_filename, argv[optind]);
				}
				break;
			case 'p':
				if(argv[optind]) {
					ss++;
					sscanf(argv[optind], "%d", &PORT);
				}
				break;
			case 'o':
				if(argv[optind]) {
					ss++;
					strcpy(output_filename, argv[optind]);
				}
				/* opens file	*/
				fd_out = open(output_filename, O_WRONLY | O_CREAT, 0666); 
				if(fd_out == -1) {
					errno = EEXIST;
					perror("open->outputfile: ");
					strcat(fd_write,"The output file can not opened!\n");
					write(fd_out, fd_write, strlen(fd_write));
					exit(EXIT_FAILURE);
				}
				break;
			case 's':
				if(argv[optind]) {
					ss++;
					sscanf(argv[optind], "%d", &startup_thread);
				}
				break;
			case 'x':
				if(argv[optind]) {
					ss++;
					sscanf(argv[optind], "%d", &maximum_thread);
				}
				break;
			case 'r':
				if(argv[optind]) {
					flag_r = 1;
					sscanf(argv[optind], "%d", &priority);
					if(priority != 0 && priority != 1 && priority != 2){
						printf("r must be equal 0, 1 or 2.\n");
						free(input_filename);
						free(output_filename);
						exit(EXIT_FAILURE);
					}	
				break;				
					}
			default:
				errno = E2BIG;
				/* if some ekstra thing is given by command line, print message with errno	*/
				perror("Extra argument for command line: ");
				strcat(fd_write,"The command line should be:./server -i pathToFile -p PORT -o pathToLogFile -s 4 -x 24\n");
				write(fd_out, fd_write, strlen(fd_write));
				free(input_filename);
				free(output_filename);				
				exit(EXIT_FAILURE);
				break;
		}	
	}
	if(flag_r==0 && argv[11]) {
		errno = E2BIG;
		/* if some ekstra thing is given by command line, print message with errno	*/
		perror("Extra argument for command line: ");
		strcat(fd_write,"The command line should be:./server -i pathToFile -p PORT -o pathToLogFile -s 4 -x 24\n");
		write(fd_out, fd_write, strlen(fd_write));
		free(input_filename);
		free(output_filename);		
		exit(EXIT_FAILURE);
	}

	/* if the parameters is missing, prints usage error	*/
	if(ss != 5) {
		errno = ENOENT;
		perror("The file is not given: ");
		strcat(fd_write,"The command line should be:./server -i pathToFile -p PORT -o pathToLogFile -s 4 -x 24 \n");
		write(fd_out, fd_write, strlen(fd_write)); 
		free(input_filename);
		free(output_filename);		
		exit(EXIT_FAILURE);
	}

	text_file = (char*)malloc(3000*sizeof(char));

	memset(fd_write, '\0', 300);
	memset(text_file, '\0', 3000);
	memset(string_thread, '\0', 1000);

	timestamp();
	//printf("Executing with parameters:\n");
	strcat(text_file, "Executing with parameters:\n");
	write(fd_out, text_file, strlen(text_file));
	//printf("text_file: %s\n", text_file);
	memset(text_file, '\0', strlen(text_file));

	

	timestamp();
	//printf("-i %s\n", input_filename);
	strcat(text_file, "-i ");
	strcat(text_file, input_filename);
	strcat(text_file, "\n");
	write(fd_out, text_file, strlen(text_file));
	//printf("text_file: %s\n", text_file);
	memset(text_file, '\0', strlen(text_file));



	timestamp();
	//printf("-p %d\n", PORT);
	strcat(text_file, "-p ");
	sprintf(string_thread, "%d", PORT);
	strcat(text_file, string_thread);
	strcat(text_file, "\n");
	write(fd_out, text_file, strlen(text_file));
	//printf("text_file: %s\n", text_file);
	memset(text_file, '\0', strlen(text_file));

	

	timestamp();
	//printf("-o %s\n", output_filename);
	strcat(text_file, "-o ");
	strcat(text_file, output_filename);
	strcat(text_file, "\n");
	write(fd_out, text_file, strlen(text_file));
	//printf("text_file: %s\n", text_file);
	memset(text_file, '\0', strlen(text_file));


	timestamp();
	//printf("-s %d\n", startup_thread);
	strcat(text_file, "-s ");
	sprintf(string_thread, "%d", startup_thread);
	strcat(text_file, string_thread);
	strcat(text_file, "\n");
	write(fd_out, text_file, strlen(text_file));
	//printf("text_file: %s\n", text_file);
	memset(text_file, '\0', strlen(text_file));


	timestamp();
	//printf("-x %d\n", maximum_thread);
	strcat(text_file, "-x ");
	sprintf(string_thread, "%d", maximum_thread);
	strcat(text_file, string_thread);
	strcat(text_file, "\n");
	write(fd_out, text_file, strlen(text_file));
	//printf("text_file: %s\n", text_file);
	memset(text_file, '\0', strlen(text_file));



	timestamp();
	//printf("Loading graph…\n");
	strcat(text_file, "Loading graph…\n");
	write(fd_out, text_file, strlen(text_file));
	//printf("text_file: %s\n", text_file);
	memset(text_file, '\0', strlen(text_file));


	//clock_t begin = clock();
	struct timeval begin, end;
	gettimeofday(&begin, NULL);



	/* opens file	*/
	fd_in = open(input_filename, O_RDONLY);
	if(fd_in == -1) {
		errno = EEXIST;
		perror("open->inputfile: ");
		strcat(fd_write,"The input file can not opened!\n");
		write(fd_out, fd_write, strlen(fd_write));  
		exit(EXIT_FAILURE);
	}

	
	nodes = (int*)malloc(count_nodes*sizeof(int));
	array_from = (int*)malloc(count_arrays*sizeof(int));
	array_to = (int*)malloc(count_arrays*sizeof(int));
	/* keeps the numbers of nodes and edges */
	numbersNodeEdge(fd_in);
	close(fd_in);


	createGraph(max_num-min_num+1);
	becomeDaemon();

	// create the graph given in above fugure 
	for(i=0; i<ind; i++) {
		addEdge(array_from[i], array_to[i]); 
	}
	gettimeofday(&end, NULL);
	float time_spent = ((end.tv_sec * 1000000 + end.tv_usec) - (begin.tv_sec * 1000000 + begin.tv_usec))*1.0 / 1000000.0;
	timestamp();
	//printf("Graph loaded in %.1f seconds with %d nodes and %d edges.\n", time_spent, index_nodes, ind);
	strcat(text_file, "Graph loaded in ");
	sprintf(string_thread, "%.1f", time_spent);
	strcat(text_file, string_thread);
	strcat(text_file, " seconds with ");
	sprintf(string_thread, "%d", index_nodes);
	strcat(text_file, string_thread);
	strcat(text_file, " nodes and ");
	sprintf(string_thread, "%d", ind);
	strcat(text_file, string_thread);
	strcat(text_file, " edges.\n");
	write(fd_out, text_file, strlen(text_file));
	//printf("text_file: %s\n", text_file);
	memset(text_file, '\0', strlen(text_file));



	initialize_cache();


	// threads  
	int additional_id = 0;
	pthread_t additional_thread;

	if (pthread_create(&additional_thread, NULL, additional, &additional_id) != 0) {
		perror("pthread_create->additional_thread: ");
		strcat(fd_write,"Insufficient resources to create thread!\n");
		write(fd_out, fd_write, strlen(fd_write));
		exit(EXIT_FAILURE);
	}




	size_thread = startup_thread;

	
	ids = (int*) malloc(maximum_thread*sizeof(int));
	threads = (pthread_t*) malloc(maximum_thread*sizeof(pthread_t));

	// threads created 
	for(i=0; i<size_thread; i++) {
		ids[i] = i;
		if (pthread_create(&threads[i], NULL, pool, &ids[i]) != 0) {
			perror("pthread_create->threads: ");
			strcat(fd_write,"Insufficient resources to create thread!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
	}



	//Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1) {
		perror("socket->socket_desc");
		strcat(fd_write,"The socket socket_disc could not created!\n");
		write(fd_out, fd_write, strlen(fd_write));
		exit(EXIT_FAILURE);
	}

	memset(&remote, 0, sizeof(struct sockaddr_in));
	remote.sin_family = AF_INET;
	remote.sin_port = htons(PORT);

	//Bind
	
	if(bind(socket_desc,(struct sockaddr *)&remote,sizeof(remote)) < 0) {
		perror("bind: ");
		strcat(fd_write,"Bind could not created!\n");
		write(fd_out, fd_write, strlen(fd_write));
		exit(EXIT_FAILURE);
	}
	listen(socket_desc, 10);




	while(1) {
		//printf("main mutex_main\n");
		if(pthread_mutex_lock(&mutex_main) != 0) {
			perror("pthread_mutex_lock->mutex_main:");
			strcat(fd_write,"The mutex mutex_main could not locked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		//printf("main mutex_main locked\n");
		clientLen = sizeof(struct sockaddr_in);
		//accept connection from an incoming client
		sock = accept(socket_desc,(struct sockaddr *)&client,(socklen_t*)&clientLen);
		if (sock < 0 && errno != EINTR) {
			perror("accept: ");
			strcat(fd_write,"The accept could not arrived or got!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}

		if(exit_signal == 0 && sock != -1){
			//printf("break server1\n");
			if(pthread_mutex_unlock(&mutex_main) != 0) {
				perror("pthread_mutex_lock->mutex_main:");
				strcat(fd_write,"The mutex mutex_main could not locked!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}	
			
			break;		
		}
		else if(exit_signal == 0 && sock == -1){
			//printf("break server1\n");

			if(pthread_cond_signal(&cond_thread) != 0) {
				perror("pthread_cond_signal->cond_thread:");
				strcat(fd_write,"Condition cond_thread could not sent!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}	
			if(pthread_cond_signal(&cond_additional) != 0) {
				perror("pthread_cond_signal->cond_thread:");
				strcat(fd_write,"Condition cond_thread could not sent!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}					
			if(pthread_mutex_unlock(&mutex_main) != 0) {
				perror("pthread_mutex_lock->mutex_main:");
				strcat(fd_write,"The mutex mutex_main could not locked!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}	
			
			break;		
		}	
		//printf("---------------\n");
		if(pthread_cond_signal(&cond_thread) != 0) {
			perror("pthread_cond_signal->cond_thread:");
			strcat(fd_write,"Condition cond_thread could not sent!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		if(pthread_cond_wait(&cond_start, &mutex_main) != 0) {
			perror("pthread_cond_wait->cond_start->mutex_main:");
			strcat(fd_write,"Condition cond_start could not arrived!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		if(pthread_mutex_unlock(&mutex_main) != 0) {
			perror("pthread_mutex_unlock->mutex_main:");
			strcat(fd_write,"Mutex mutex_main could not unlocked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		//printf("iiiiiiiiiiiiii\n");
		/*if(pthread_mutex_lock(&mutex_main) != 0) {
			perror("pthread_mutex_lock->mutex_main:");
			strcat(fd_write,"The mutex mutex_main could not locked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		//printf("şşşşşşşşşşşşşşşşşş\n");
		//printf("jjjjjjjjjjjjjjj\n");
		if(pthread_mutex_unlock(&mutex_main) != 0) {
			perror("pthread_mutex_unlock->mutex_main:");
			strcat(fd_write,"The mutex mutex_main could not unlocked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}*/
		if(pthread_mutex_lock(&mutex_additional) != 0) {
			perror("pthread_mutex_lock->mutex_additional:");
			strcat(fd_write,"The mutex mutex_additional could not locked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		//printf("///////////////////////\n");
		while(usage_thread == size_thread) {
			//printf("usage = size_thread\n");
				strcat(text_file, "No thread is available! Waiting for one.\n");
				write(fd_out, text_file, strlen(text_file));
				//printf("text_file: %s\n", text_file);
				memset(text_file, '\0', strlen(text_file));			

			if(pthread_cond_wait(&cond_waiting, &mutex_additional) != 0) {
				perror("pthread_cond_wait->cond_waiting->mutex_additional:");
				strcat(fd_write,"Condition cond_waiting could not arrived!\n");
				write(fd_out, fd_write, strlen(fd_write));
				exit(EXIT_FAILURE);
			}
		}
		//printf("0000000000000000000\n");
		if(pthread_mutex_unlock(&mutex_additional) != 0) {
			perror("pthread_mutex_unlock->mutex_additional:");
			strcat(fd_write,"The mutex mutex_additional could not unlocked!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
		//printf("11111111111111111111111\n");
		
	}

	for (int i = 0; i < startup_thread; ++i) {
		if(pthread_join(threads[i], NULL) == -1) {
			perror("pthread_join->threads:");
			strcat(fd_write,"Thread could not killed!\n");
			write(fd_out, fd_write, strlen(fd_write));
			exit(EXIT_FAILURE);
		}
	}
	if(pthread_join(additional_thread, NULL) == -1) {
		perror("pthread_join->threads:");
		strcat(fd_write,"Thread could not killed!\n");
		write(fd_out, fd_write, strlen(fd_write));
		exit(EXIT_FAILURE);
	}

	strcat(text_file, "All threads have terminated, server shutting down.\n");
	write(fd_out, text_file, strlen(text_file));
	//printf("text_file: %s\n", text_file);
	memset(text_file, '\0', strlen(text_file));	

	free(nodes);
	free(array_from);
	free(array_to);

	free(input_filename);
	free(output_filename);
	free(fd_write);
	free(text_file);
	free(ids);
	free(threads);

	
	for (i = 0; i < max_num-min_num+1; ++i) {
		
		//printf("i=%d\n", i);
		for(int j=0; j<max_num-min_num+1; j++) {
			//printf("graph->array[i].head[j].next: %s\n", graph->array[i].head[j].next );
			//printf("j=%d     ", j);
			//if(graph->array[i].head[j].next != NULL)
				free(graph->array[i].head[j].next);
		}
		///printf("---\n");
		
		free(graph->array[i].head);
	}
	
	free(graph->array);
	free(graph->visited);
	free(graph);
	for(i=0; i<index_nodes*index_nodes; i++) {
		free(cache[i].path);
	}
	free(cache);

	return 0;
}