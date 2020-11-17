#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
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

// handlere for SIGINT
void handler(int sig);

// shared memory buffer
struct sembuf
{
	// to know supplier's plates
	sem_t items_supplier_soup;
	sem_t items_supplier_maincourse;
	sem_t items_supplier_dessert;
	// to know about plates in kitchen
	sem_t items_kitchen_soup;
	sem_t items_kitchen_maincourse;
	sem_t items_kitchen_dessert;
	// to know about plates in counter
	sem_t items_counter_soup;
	sem_t items_counter_maincourse;
	sem_t items_counter_dessert;
	// kitchen size and number of plates in kitchen 
	sem_t kitchen_size;
	sem_t kitchen_item_size;
	// counter size
	sem_t counter_size;
	// table size
	sem_t table;
	// binary semaphores
	sem_t mutex;
	sem_t mutex3;
	// the number of students where in counter
	sem_t students_counter;
	// to keep round
	int* round;
	// to know finished processes
	int count;
	// to manage supplier's work
	int flag;
	// to manage tables
	int* table_index;
	// count of students where in counter or not
	int* counter_stu;
	int* counter_cook;

};

// id of parent process
sig_atomic_t parent_pid;
sig_atomic_t numberof_processes;

// to create semaphore 
struct sembuf *sem1;
int fd, fd_in;
char* num;
char* input_filename;



int main(int argc, char* argv[]) {

	// in case of ctrl c exitted gracefully
	struct sigaction sa;
	if(sigemptyset(&sa.sa_mask) == -1) {
		perror("Sigemptyset");
		exit(EXIT_FAILURE);
	}
	sa.sa_flags = 0;
	sa.sa_handler = handler;
	
	if(sigaction(SIGINT, &sa, NULL) == -1){
		perror("Sigaction");
		exit(EXIT_FAILURE);
	}

	parent_pid = getpid();
	int opt, time_temp=0, rea;

	num = (char*)malloc(1000*sizeof(char));
	input_filename = (char*)malloc(20*sizeof(char));

	int n, m, t, s, l, k;
	while((opt = getopt(argc, argv, "NMTSLF")) != -1)	
	{	
		switch(opt)	
		{	
			case 'N':
				if(!argv[optind]) {
					perror("You missed number of cook. Check your commend!\n");
					exit(EXIT_FAILURE);
				}
				num = argv[optind];
				sscanf(num, "%d", &n);
				if(n<=2) {
					perror("Should be N>2. Check your value!\n");
					exit(EXIT_FAILURE);
				}
				break;	
			case 'M':
				if(!argv[optind]) {
					perror("You missed number of student. Check your commend!\n");
					exit(EXIT_FAILURE);
				}
				num = argv[optind];
				sscanf(num, "%d", &m);
				if(m<=n) {
					perror("Should be M>N. Check your value!\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'T':
				if(!argv[optind]) {
					perror("You missed number of table. Check your commend!\n");
					exit(EXIT_FAILURE);
				}
				num = argv[optind];
				sscanf(num, "%d", &t);				
				if(t<1) {
					perror("Should be T>=1. Check your value!\n");
					exit(EXIT_FAILURE);
				}
				if(m<=t) {
					perror("Should be M>T. Check your value!\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'S':
				if(!argv[optind]) {
					perror("You missed a counter of size S. Check your commend!\n");
					exit(EXIT_FAILURE);
				}
				num = argv[optind];
				sscanf(num, "%d", &s);				
				if(s<=3) {
					perror("Should be S>3. Check your value!\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'L':
				if(!argv[optind]) {
					perror("You missed number of L (round). Check your commend!\n");
					exit(EXIT_FAILURE);
				}
				num = argv[optind];
				sscanf(num, "%d", &l);				
				if(l<3) {
					perror("Should be L>=3. Check your value!\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'F':
				input_filename = argv[optind];
				break;
			default:
				/* if some ekstra thing is given by commend line, print message with errno */
				perror("The commond line should be:\n./program -N 3 -M 12 -T 5 -S 4 -L 13 -F filePath\n");
				exit(EXIT_FAILURE);
				break;
		}	
	}
	k = (2*l*m) + 1;
	numberof_processes = m+n+1;

	// created semaphore 
	const char* name = "SP";
	fd = shm_open(name, O_CREAT | O_RDWR, 0666);
	if (fd == -1) {
		perror("shm_open\n");
		exit(EXIT_FAILURE);
	}
	if (ftruncate(fd, sizeof(struct sembuf)) == -1) {
		perror("ftruncate\n");
		exit(EXIT_FAILURE);
	}

	sem1 = mmap(NULL, sizeof(struct sembuf), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (sem1 == MAP_FAILED) {
		perror("mmap\n");	
		exit(EXIT_FAILURE);
	}

	// initialized this values
	int numberofplate = l*m;

	if(sem_init(&sem1->items_supplier_soup, 1, numberofplate) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sem1->items_supplier_maincourse, 1, numberofplate) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sem1->items_supplier_dessert, 1, numberofplate) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sem1->items_kitchen_soup, 1, 0) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sem1->items_kitchen_maincourse, 1, 0) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sem1->items_kitchen_dessert, 1, 0) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sem1->items_counter_soup, 1, 0) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sem1->items_counter_maincourse, 1, 0) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sem1->items_counter_dessert, 1, 0) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sem1->kitchen_size, 1, k) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sem1->kitchen_item_size, 1, 0) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sem1->counter_size, 1, s) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sem1->table, 1, t) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sem1->mutex, 1, 1) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sem1->mutex3, 1, 1) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sem1->students_counter, 1, 0) == -1) {
		perror("sem_init");
		exit(EXIT_FAILURE);
	}
	sem1->count = 0;
	sem1->flag = 0;
	char buffer[3*l*m];

	sem1->table_index = (int*) malloc(t * sizeof(int));
	for(int i=0; i<t; i++)
		sem1->table_index[i] = 0;
	srand(time(NULL));

	sem1->counter_stu = (int*) malloc(m * sizeof(int));
	for(int i=0; i<m; i++)
		sem1->counter_stu[i] = 0;

	sem1->counter_cook = (int*) malloc(n * sizeof(int));
	for(int i=0; i<n; i++)
		sem1->counter_cook[i] = 0;


	sem1->round = (int*) malloc(m * sizeof(int));
	for(int i=0; i<m; i++)
		sem1->round[i] = 1;

	// used in supplier, cook, student
	int v, v1, v2, v3;
	int c, c1, c2, c3;
	int stu1, stu2, stu3, student;
	int depo_cook, depo_supplier=1, depo_student, d1, d2, d3;
	int children[n+m+1];
	for(int i=0; i<=n; i++) {
		children[i] = 0;
	}

	for(int i=0; i<(n+m+1); i++) {
		if(getpid() == parent_pid) {
			switch (fork()){
				case -1: // if there is any problem to create any child
					perror("fork");
					exit(EXIT_FAILURE);
				case 0:  // child
					children[i]=getpid();
					break;
				default: // parent
					break;
			}
		}
	}
	// executes infinite times
	while(1) {

		// if the process is supplier
		if(sem1->flag==0 && depo_supplier!=0 && children[0] == getpid()) {

			fd_in = open(input_filename, O_RDONLY);
			if(depo_supplier != 0 && fd_in == -1) {
				perror("open");
				exit(EXIT_FAILURE);
			}

			char onechar[2];
			for(int ind=0; ind<3*l*m;ind++) {

				int read_success = read(fd_in,onechar,1);
				if(depo_supplier != 0 && read_success==-1){
					perror("read");
					exit(EXIT_FAILURE);
				}
				// if there is no new line, keep character   
				if(onechar[0] != '\n') {
					
					// delivers plates to kitchen while there is some plates and there is some places in kitchen 
					if(sem_wait(&sem1->mutex3) == -1) {
						perror("sem_wait");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->items_supplier_soup, &d1) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->items_supplier_maincourse, &d2) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->items_supplier_dessert, &d3) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					depo_supplier = d1+d2+d3;
					if(sem_post(&sem1->mutex3) == -1) {
						perror("sem_post");
						exit(EXIT_FAILURE);
					}
					// if plate is SOUP 
					if(onechar[0] == 'P') {
						
						if(sem_wait(&sem1->mutex) == -1) {
							perror("sem_wait");
							exit(EXIT_FAILURE);
						}
						// if there is empty place, deliver soup
						if(sem_getvalue(&sem1->kitchen_size, &v) == -1) {
							perror("sem_getvalue");
							exit(EXIT_FAILURE);
						}
						if(v>0) {
							if(sem_wait(&sem1->kitchen_size) == -1) {
								perror("sem_wait");
								exit(EXIT_FAILURE);
							}
							// if supplier has soup, deliver it kitchen
							if(sem_getvalue(&sem1->items_supplier_soup, &v) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							if(v>0) {
								if(sem_getvalue(&sem1->items_kitchen_soup, &v1) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								if(sem_getvalue(&sem1->items_kitchen_maincourse, &v2) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								if(sem_getvalue(&sem1->items_kitchen_dessert, &v3) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								v = v1+v2+v3;
								// Entering the kitchen
								printf("The supplier is going to the kitchen to deliver soup: kitchen items: P:%d, C:%d, D:%d = %d\n", v1, v2, v3, v);
								// size of soup of supplier is decrease, sizes of item and soup of kitchen is increase
								if(sem_wait(&sem1->items_supplier_soup) == -1) {
									perror("sem_wait");
									exit(EXIT_FAILURE);
								}
								if(sem_post(&sem1->items_kitchen_soup) == -1) {
									perror("sem_post");
									exit(EXIT_FAILURE);
								}
								if(sem_post(&sem1->kitchen_item_size) == -1) {
									perror("sem_post");
									exit(EXIT_FAILURE);
								}
								if(sem_getvalue(&sem1->items_kitchen_soup, &v1) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								if(sem_getvalue(&sem1->items_kitchen_maincourse, &v2) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								if(sem_getvalue(&sem1->items_kitchen_dessert, &v3) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								v = v1+v2+v3;
								// Afer delivery
								printf("The supplier delivered soup – after delivery: kitchen items: P:%d, C:%d, D:%d = %d\n", v1, v2, v3, v);
							}
							else 
								if(sem_post(&sem1->kitchen_size) == -1) {
									perror("sem_post");
									exit(EXIT_FAILURE);
								}
						}
						if(sem_post(&sem1->mutex) == -1) {
							perror("sem_post");
							exit(EXIT_FAILURE);
						}
					}

					// if plate is MAIN COURSE
					if(onechar[0] == 'C') {
						if(sem_wait(&sem1->mutex) == -1) {
							perror("sem_wait");
							exit(EXIT_FAILURE);
						}
						// if there is empty place, deliver main course
						if(sem_getvalue(&sem1->kitchen_size, &v) == -1) {
							perror("sem_getvalue");
							exit(EXIT_FAILURE);
						}
						if(v>0) {
							if(sem_wait(&sem1->kitchen_size) == -1) {
								perror("sem_wait");
								exit(EXIT_FAILURE);
							}
							// if supplier has main course, deliver it kitchen
							if(sem_getvalue(&sem1->items_supplier_maincourse, &v) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							if(v>0) {
								if(sem_getvalue(&sem1->items_kitchen_soup, &v1) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								if(sem_getvalue(&sem1->items_kitchen_maincourse, &v2) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								if(sem_getvalue(&sem1->items_kitchen_dessert, &v3) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								v = v1+v2+v3;
								// Entering the kitchen
								printf("The supplier is going to the kitchen to deliver main course: kitchen items: P:%d, C:%d, D:%d = %d\n", v1, v2, v3, v);
								// size of main course of supplier is decrease, sizes of item and main course of kitchen is increase
								if(sem_wait(&sem1->items_supplier_maincourse) == -1) {
									perror("sem_wait");
									exit(EXIT_FAILURE);
								}
								if(sem_post(&sem1->items_kitchen_maincourse) == -1) {
									perror("sem_post");
									exit(EXIT_FAILURE);
								}
								// mutfaktaki urun sayisi artiyor
								if(sem_post(&sem1->kitchen_item_size) == -1) {
									perror("sem_post");
									exit(EXIT_FAILURE);
								}
								if(sem_getvalue(&sem1->items_kitchen_soup, &v1) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								if(sem_getvalue(&sem1->items_kitchen_maincourse, &v2) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								if(sem_getvalue(&sem1->items_kitchen_dessert, &v3) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								v = v1+v2+v3;
								// Afer delivery
								printf("The supplier delivered main course – after delivery: kitchen items: P:%d, C:%d, D:%d = %d\n", v1, v2, v3, v);
							}
							else 
								if(sem_post(&sem1->kitchen_size) == -1) {
									perror("sem_post");
									exit(EXIT_FAILURE);
								}
						}
						if(sem_post(&sem1->mutex) == -1) {
							perror("sem_post");
							exit(EXIT_FAILURE);
						}
					}
					
					// if plate is DESSERT
					if(onechar[0] == 'D') {
						if(sem_wait(&sem1->mutex) == -1) {
							perror("sem_wait");
							exit(EXIT_FAILURE);
						}
						// if there is empty place, deliver dessert
						if(sem_getvalue(&sem1->kitchen_size, &v) == -1) {
							perror("sem_getvalue");
							exit(EXIT_FAILURE);
						}
						if(v>0) {
							if(sem_wait(&sem1->kitchen_size) == -1) {
								perror("sem_wait");
								exit(EXIT_FAILURE);
							}
							// if supplier has dessert, deliver it kitchen
							if(sem_getvalue(&sem1->items_supplier_dessert, &v) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							if(v>0) {
								if(sem_getvalue(&sem1->items_kitchen_soup, &v1) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								if(sem_getvalue(&sem1->items_kitchen_maincourse, &v2) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								if(sem_getvalue(&sem1->items_kitchen_dessert, &v3) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								v = v1+v2+v3;
								// Entering the kitchen
								printf("The supplier is going to the kitchen to deliver dessert: kitchen items: P:%d, C:%d, D:%d = %d\n", v1, v2, v3, v);
								// size of dessert of supplier is decrease, sizes of item and dessert of kitchen is increase
								if(sem_wait(&sem1->items_supplier_dessert) == -1) {
									perror("sem_wait");
									exit(EXIT_FAILURE);
								}
								if(sem_post(&sem1->items_kitchen_dessert) == -1) {
									perror("sem_post");
									exit(EXIT_FAILURE);
								}
								// mutfaktaki urun sayisi artiyor
								if(sem_post(&sem1->kitchen_item_size) == -1) {
									perror("sem_post");
									exit(EXIT_FAILURE);
								}
								if(sem_getvalue(&sem1->items_kitchen_soup, &v1) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								if(sem_getvalue(&sem1->items_kitchen_maincourse, &v2) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								if(sem_getvalue(&sem1->items_kitchen_dessert, &v3) == -1) {
									perror("sem_getvalue");
									exit(EXIT_FAILURE);
								}
								v = v1+v2+v3;
								// Afer delivery
								printf("The supplier delivered dessert – after delivery: kitchen items: P:%d, C:%d, D:%d = %d\n", v1, v2, v3, v);
							}
							else 
								if(sem_post(&sem1->kitchen_size) == -1) {
									perror("sem_post");
									exit(EXIT_FAILURE);
								}
						}
						if(sem_post(&sem1->mutex) == -1) {
							perror("sem_post");
							exit(EXIT_FAILURE);
						}
					}

					if(sem_wait(&sem1->mutex3) == -1) {
						perror("sem_wait");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->items_supplier_soup, &d1) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->items_supplier_maincourse, &d2) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->items_supplier_dessert, &d3) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					depo_supplier = d1+d2+d3;
					if(sem1->flag==0 && depo_supplier == 0) {
						// Done delivering.
						printf("The supplier finished supplying – GOODBYE!\n");
						close(fd_in);
						sem1->flag=1;
						(sem1->count)++;
					}
					if(sem_post(&sem1->mutex3) == -1) {
						perror("sem_post");
						exit(EXIT_FAILURE);
					}
				}
				else
					ind--;
			}	
		}

		// if the process is cook
		for(int i=1; i<=n; i++) {
			if(children[i] == getpid()) {

				if(sem_wait(&sem1->mutex3) == -1) {
					perror("sem_wait");
					exit(EXIT_FAILURE);
				}
				if(sem_getvalue(&sem1->items_kitchen_soup, &v1) == -1) {
					perror("sem_getvalue");
					exit(EXIT_FAILURE);
				}
				if(sem_getvalue(&sem1->items_kitchen_maincourse, &v2) == -1) {
					perror("sem_getvalue");
					exit(EXIT_FAILURE);
				}
				if(sem_getvalue(&sem1->items_kitchen_dessert, &v3) == -1) {
					perror("sem_getvalue");
					exit(EXIT_FAILURE);
				}
				depo_cook = v1+v2+v3;
				if(sem_post(&sem1->mutex3) == -1) {
					perror("sem_post");
					exit(EXIT_FAILURE);
				}
				// place the plates while there is some plates of course 
				while(depo_cook>0) {

					if(sem_wait(&sem1->mutex) == -1) {
						perror("sem_wait");
						exit(EXIT_FAILURE);
					}
					if(sem1->counter_cook[i-1] == 0) {
						printf("Cook %d is going to the kitchen to wait for/get a plate - kitchen items P:%d, C:%d, D:%d = %d\n", i-1, v1, v2, v3, depo_cook);
						sem1->counter_cook[i-1] = 1;
					}
					// place the soup plates if there is some empty place in counter
					if(sem_getvalue(&sem1->counter_size, &c) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					if(c>0) {
						if(sem_wait(&sem1->counter_size) == -1) {
							perror("sem_wait");
							exit(EXIT_FAILURE);
						}
						if(sem_getvalue(&sem1->items_kitchen_soup, &c1) == -1) {
							perror("sem_getvalue");
							exit(EXIT_FAILURE);
						}
						if(sem_getvalue(&sem1->items_counter_soup, &c2) == -1) {
							perror("sem_getvalue");
							exit(EXIT_FAILURE);
						}
						if(c1>0 && c2<(s/3)) {
							// size of soups and size of items is increase in counter while size of counter
							if(sem_getvalue(&sem1->items_counter_soup, &c1) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							if(sem_getvalue(&sem1->items_counter_maincourse, &c2) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							if(sem_getvalue(&sem1->items_counter_dessert, &c3) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							c = c1+c2+c3;
							// Going to the counter 
							printf("Cook %d is going to the counter to deliver soup – counter items P:%d, C:%d, D:%d = %d\n", i-1, c1, c2, c3, c);

							if(sem_wait(&sem1->items_kitchen_soup) == -1) {
								perror("sem_wait");
								exit(EXIT_FAILURE);
							}
							if(sem_post(&sem1->items_counter_soup) == -1) {
								perror("sem_post");
								exit(EXIT_FAILURE);
							}

							// size of soups and size of items are decrease in kitchen 
							if(sem_wait(&sem1->kitchen_item_size) == -1) {
								perror("sem_wait");
								exit(EXIT_FAILURE);
							}
							if(sem_post(&sem1->kitchen_size) == -1) {
								perror("sem_post");
								exit(EXIT_FAILURE);
							}
							// size of soups and size of items is increase in counter while size of counter
							if(sem_getvalue(&sem1->items_counter_soup, &c1) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							if(sem_getvalue(&sem1->items_counter_maincourse, &c2) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							if(sem_getvalue(&sem1->items_counter_dessert, &c3) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							c = c1+c2+c3;
							// After delivery to counter 
							printf("Cook %d placed soup on the counter - counter items: P:%d, C:%d, D:%d = %d\n", i-1, c1, c2, c3, c);
							sem1->counter_cook[i-1]=0;
						}
						else
							if(sem_post(&sem1->counter_size) == -1) {
								perror("sem_post");
								exit(EXIT_FAILURE);
							}
					}
					if(sem_post(&sem1->mutex) == -1) {
						perror("sem_post");
						exit(EXIT_FAILURE);
					}


					if(sem_wait(&sem1->mutex) == -1) {
						perror("sem_wait");
						exit(EXIT_FAILURE);
					}
					// place the main course plates if there is some empty place in counter
					if(sem_getvalue(&sem1->counter_size, &c) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					if(c>0) {
						if(sem_wait(&sem1->counter_size) == -1) {
							perror("sem_wait");
							exit(EXIT_FAILURE);
						}
						if(sem_getvalue(&sem1->items_kitchen_maincourse, &c1) == -1) {
							perror("sem_getvalue");
							exit(EXIT_FAILURE);
						}
						if(sem_getvalue(&sem1->items_counter_maincourse, &c2) == -1) {
							perror("sem_getvalue");
							exit(EXIT_FAILURE);
						}
						if(c1>0 && c2<(s/3)) {
							// size of main course and size of items is increase in counter while size of counter
							if(sem_getvalue(&sem1->items_counter_soup, &c1) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							if(sem_getvalue(&sem1->items_counter_maincourse, &c2) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							if(sem_getvalue(&sem1->items_counter_dessert, &c3) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							c = c1+c2+c3;
							// Going to the counter 
							printf("Cook %d is going to the counter to deliver main course – counter items P:%d, C:%d, D:%d = %d\n", i-1, c1, c2, c3, c);

							if(sem_wait(&sem1->items_kitchen_maincourse) == -1) {
								perror("sem_wait");
								exit(EXIT_FAILURE);
							}
							if(sem_post(&sem1->items_counter_maincourse) == -1) {
								perror("sem_post");
								exit(EXIT_FAILURE);
							}
							// size of main courses and size of items are decrease in kitchen 
							if(sem_wait(&sem1->kitchen_item_size) == -1) {
								perror("sem_wait");
								exit(EXIT_FAILURE);
							}
							if(sem_post(&sem1->kitchen_size) == -1) {
								perror("sem_post");
								exit(EXIT_FAILURE);
							}
							// size of main courses and size of items is increase in counter while size of counter
							if(sem_getvalue(&sem1->items_counter_soup, &c1) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							if(sem_getvalue(&sem1->items_counter_maincourse, &c2) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							if(sem_getvalue(&sem1->items_counter_dessert, &c3) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							c = c1+c2+c3;
							// After delivery to counter 
							printf("Cook %d placed main course on the counter - counter items: P:%d, C:%d, D:%d = %d\n", i, c1, c2, c3, c);
							sem1->counter_cook[i-1]=0;
						}
						else
							if(sem_post(&sem1->counter_size) == -1) {
								perror("sem_post");
								exit(EXIT_FAILURE);
							}
					}
					if(sem_post(&sem1->mutex) == -1) {
						perror("sem_post");
						exit(EXIT_FAILURE);
					}

					if(sem_wait(&sem1->mutex) == -1) {
						perror("sem_wait");
						exit(EXIT_FAILURE);
					}
					// place the dessert plates if there is some empty place in counter
					if(sem_getvalue(&sem1->counter_size, &c) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					if(c>0) {
						if(sem_wait(&sem1->counter_size) == -1) {
							perror("sem_wait");
							exit(EXIT_FAILURE);
						}
						if(sem_getvalue(&sem1->items_kitchen_dessert, &c1) == -1) {
							perror("sem_getvalue");
							exit(EXIT_FAILURE);
						}
						if(sem_getvalue(&sem1->items_counter_dessert, &c2) == -1) {
							perror("sem_getvalue");
							exit(EXIT_FAILURE);
						}
						if(c1>0 && c2<(s/3)) {
							// size of dessert and size of items is increase in counter while size of counter
							if(sem_getvalue(&sem1->items_counter_soup, &c1) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							if(sem_getvalue(&sem1->items_counter_maincourse, &c2) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							if(sem_getvalue(&sem1->items_counter_dessert, &c3) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							c = c1+c2+c3;
							// Going to the counter 
							printf("Cook %d is going to the counter to deliver dessert – counter items P:%d, C:%d, D:%d = %d\n", i-1, c1, c2, c3, c);

							if(sem_wait(&sem1->items_kitchen_dessert) == -1) {
								perror("sem_wait");
								exit(EXIT_FAILURE);
							}
							if(sem_post(&sem1->items_counter_dessert) == -1) {
								perror("sem_post");
								exit(EXIT_FAILURE);
							}
							// size of desserts and size of items are decrease in kitchen 
							if(sem_wait(&sem1->kitchen_item_size) == -1) {
								perror("sem_wait");
								exit(EXIT_FAILURE);
							}
							if(sem_post(&sem1->kitchen_size) == -1) {
								perror("sem_post");
								exit(EXIT_FAILURE);
							}
							// size of desserts and size of items is increase in counter while size of counter
							if(sem_getvalue(&sem1->items_counter_soup, &c1) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							if(sem_getvalue(&sem1->items_counter_maincourse, &c2) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							if(sem_getvalue(&sem1->items_counter_dessert, &c3) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							c = c1+c2+c3;
							// After delivery to counter 
							printf("Cook %d placed dessert on the counter - counter items: P:%d, C:%d, D:%d = %d\n", i, c1, c2, c3, c);
							sem1->counter_cook[i-1]=0;
						}
						else
							if(sem_post(&sem1->counter_size) == -1) {
								perror("sem_post");
								exit(EXIT_FAILURE);
							}
					}
					if(sem_post(&sem1->mutex) == -1) {
						perror("sem_post");
						exit(EXIT_FAILURE);
					}
					
					if(sem_wait(&sem1->mutex3) == -1) {
						perror("sem_wait");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->items_kitchen_soup, &v1) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->items_kitchen_maincourse, &v2) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->items_kitchen_dessert, &v3) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					depo_cook = v1+v2+v3;
					
					if(depo_cook == 0) {
						// After finishing placing all plates
						printf("Cook %d finished serving - items at kitchen: %d – going home – GOODBYE!!!\n", i, depo_cook);
						(sem1->count)++;
						if(sem_post(&sem1->mutex3) == -1) {
							perror("sem_post");
							exit(EXIT_FAILURE);
						}
						break;
					}
					if(sem_post(&sem1->mutex3) == -1) {
						perror("sem_post");
						exit(EXIT_FAILURE);
					}
				}
			}
		}

		for(int i=n+1; i<m+n+1; i++) {
			if(children[i] == getpid()) {
				int tab;
				// execute while the round of student is not equal to l
				if(sem1->round[i-n-1]<l+1 && sem1->counter_stu[i-n-1] == 0) {
					if(sem_wait(&sem1->mutex3) == -1) {
						perror("sem_wait");
						exit(EXIT_FAILURE);
					}
					sem1->counter_stu[i-n-1] = 1;
					if(sem_post(&sem1->students_counter) == -1) {
						perror("sem_post");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->items_counter_soup, &c1) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->items_counter_maincourse, &c2) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->items_counter_dessert, &c3) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->students_counter, &student) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					c = c1+c2+c3;
					// arriving at the counter/waiting for food
					printf("Student %d is going to the counter (round %d) - # of students at counter: %d and counter items: P:%d, C:%d, D:%d = %d\n", i-n-1, sem1->round[i-n-1], student, c1, c2, c3, c);
					if(sem_post(&sem1->mutex3) == -1) {
						perror("sem_post");
						exit(EXIT_FAILURE);
					}
				}

				while(sem1->counter_stu[i-n-1] == 1 && sem1->round[i-n-1] <= l) {
					// student gets foods if there are 1 soup, 1 main course and 1 dessert at least
					if(sem_wait(&sem1->mutex3) == -1) {
						perror("sem_wait");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->items_counter_soup, &stu1) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->items_counter_maincourse, &stu2) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					if(sem_getvalue(&sem1->items_counter_dessert, &stu3) == -1) {
						perror("sem_getvalue");
						exit(EXIT_FAILURE);
					}
					if(sem_post(&sem1->mutex3) == -1) {
						perror("sem_post");
						exit(EXIT_FAILURE);
					}

					if(stu1>0 && stu2>0 && stu3>0 && sem1->round[i-n-1] <= l) {

						if(sem_wait(&sem1->students_counter) == -1) {
							perror("sem_wait");
							exit(EXIT_FAILURE);
						}
						// student got foods ans decreased foods
						if(sem_wait(&sem1->items_counter_soup) == -1) {
							perror("sem_wait");
							exit(EXIT_FAILURE);
						}
						if(sem_post(&sem1->counter_size) == -1) {
							perror("sem_post");
							exit(EXIT_FAILURE);
						}
						if(sem_wait(&sem1->items_counter_maincourse) == -1) {
							perror("sem_wait");
							exit(EXIT_FAILURE);
						}
						if(sem_post(&sem1->counter_size) == -1) {
							perror("sem_post");
							exit(EXIT_FAILURE);
						}
						if(sem_wait(&sem1->items_counter_dessert) == -1) {
							perror("sem_wait");
							exit(EXIT_FAILURE);
						}
						if(sem_post(&sem1->counter_size) == -1) {
							perror("sem_post");
							exit(EXIT_FAILURE);
						}
						
						// now the search for the table is start
						if(sem_getvalue(&sem1->table, &tab) == -1) {
							perror("sem_getvalue");
							exit(EXIT_FAILURE);
						}
						if(tab>0) {
							// waiting for/getting a table
							printf("Student %d got food and is going to get a table (round %d) - # of empty tables: %d\n", i-n-1, sem1->round[i-n-1] , tab);
							int ss=0;
							for(ss=0; ss<t; ss++) {
								ss=(rand()%t)+1;
								if(sem1->table_index[ss] == 0) {
									sem1->table_index[ss] = 1;
									break;
								}
							}
							// student sat at table
							if(sem_wait(&sem1->table) == -1) {
								perror("sem_wait");
								exit(EXIT_FAILURE);
							}
							// sitting to eat
							if(sem_getvalue(&sem1->table, &tab) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							printf("Student %d sat at table %d to eat (round %d) - empty tables: %d\n", i-n-1, ss, sem1->round[i-n-1], tab);
							// student left table
							sem1->table_index[ss] = 0;
							if(sem_post(&sem1->table) == -1) {
								perror("sem_post");
								exit(EXIT_FAILURE);
							}

							// done eating, going again to the counter; times x, where x is increased to x+1
							if(sem_getvalue(&sem1->table, &tab) == -1) {
								perror("sem_getvalue");
								exit(EXIT_FAILURE);
							}
							printf("Student %d left table %d to eat again (round %d) - empty tables: %d\n", i-n-1, ss, sem1->round[i-n-1], tab);
							sem1->counter_stu[i-n-1] = 0;
							(sem1->round[i-n-1]) += 1;
							
							if(sem1->round[i-n-1]==l+1) {
								printf("Student %d is done eating L=%d times - going home – GOODBYE!!!\n", i-n-1, l);
								(sem1->count)++;
								break;
							}
						}
						break;
					}
					else
						break;
				}	
			}
		}
		if(sem1->count>=m+n+1)
			break;
	}
	// exits all the processes
	for(int i=0; i<m+n+1; i++) {
		if(getpid() != parent_pid)
			exit(0);
	}
	if(getpid() == parent_pid)
		exit(0);

	// close and free all resources
	if(close(fd) == -1) {
		perror("close");
		exit(EXIT_FAILURE);
	}
	if(shm_unlink(name) == -1) {
		perror("shm_unlink");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->items_supplier_soup) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->items_supplier_maincourse) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->items_supplier_dessert) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->items_kitchen_soup) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->items_kitchen_maincourse) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->items_kitchen_dessert) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->items_counter_soup) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->items_counter_maincourse) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->items_counter_dessert) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->kitchen_size) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->kitchen_item_size) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->counter_size) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->table) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->mutex) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->mutex3) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem1->students_counter) == -1) {
		perror("sem_destroy");
		exit(EXIT_FAILURE);
	}

	free(&sem1->round);
	free(&sem1->count);
	free(&sem1->flag);
	free(&sem1->table_index);
	free(&sem1->counter_stu);
	free(&sem1->counter_cook);

	if(munmap(sem1, sizeof(struct sembuf)) == -1){
		perror("munmap1");
		exit(EXIT_FAILURE);
	}

	free(num);
	free(input_filename);

	return 0;
}

// handler for signal SIGINT 
void handler(int sig) {
	// if signal is SIGINT, exit all the processes
	if(sig==SIGINT) {
		for(int i=0; i<numberof_processes; i++) {
			if(getpid() != parent_pid)
				exit(0);
		}
		if(getpid() == parent_pid) {
			// close and free all resources
			if(close(fd) == -1) {
				perror("close");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->items_supplier_soup) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->items_supplier_maincourse) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->items_supplier_dessert) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->items_kitchen_soup) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->items_kitchen_maincourse) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->items_kitchen_dessert) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->items_counter_soup) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->items_counter_maincourse) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->items_counter_dessert) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->kitchen_size) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->kitchen_item_size) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->counter_size) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->table) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->mutex) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->mutex3) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(sem_destroy(&sem1->students_counter) == -1) {
				perror("sem_destroy");
				exit(EXIT_FAILURE);
			}
			if(munmap(sem1, sizeof(struct sembuf)) == -1){
				perror("munmap1");
				exit(EXIT_FAILURE);
			}
			exit(0);
		}
	}
}