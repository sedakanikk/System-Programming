#include <stdio.h> 
#include <stdlib.h> 
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int power(int number, int pow) {
	int res=1;
	for(int i=0; i<pow; i++) {
		res *= number;
	}
	return res;
}

int main(int argc, char* argv[]) {

	int opt, base = 10, sleep_time=0, time_temp=0, max_size=160, i = 0, line_count=0, temp=0;
	char *input_filename, *output_filename;
	unsigned char onechar[2], buffer[max_size];
	char** char_ascii;
	char_ascii = (char**) malloc(64 * sizeof(char));
	int c;
	for ( c=0; c<64; c++ )
	{
		char_ascii[c] = (char*) malloc(9 * sizeof(char));
	}

	// ./programB -i inputPathB -o outputPathB -t time
	while((opt = getopt(argc, argv, "iot")) != -1)  
	{  
		switch(opt)  
		{  
			case 'i':  
				if(strcmp(argv[optind], "inputPathB") == 0)
					input_filename = "outputPathA";
				else
					input_filename = argv[optind];
				break;  
			case 'o':  
				output_filename = argv[optind];
				break;
			case 't':  
				while((int)argv[optind][time_temp] != 0) {
					time_temp++;
				}
				time_temp--;
				int max_ind=time_temp;
				int tt_time=0;
				while(tt_time<max_ind) {
					int f = (int)argv[optind][tt_time]-48;
					int p = power(10, time_temp);
					sleep_time += p*f;
					time_temp--;
					tt_time++;
				}
				break;
		}  
	} 
	int fd1 = open(input_filename, O_RDONLY);
	int fd2 = open(output_filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
	if (fd1 == -1 || fd2 == -1) {
		free(input_filename);
		free(output_filename);
		for ( c=0; c<64; c++ ) {
			free(char_ascii[c]);
		}
		free(char_ascii);
        return 1;
	}
	// finds line count
	while (fd1!=-1) {
		if(read(fd1,onechar,1) == 0) 
			break;
		onechar[1] = '\0'; 
		if(onechar[0] == '\n')
			line_count++;
	}
	close(fd1);

	int selected_line = rand()%line_count + 1;
	fd1 = open(input_filename, O_RDONLY);
	if (fd1 == -1) {
		free(input_filename);
		free(output_filename);
		for ( c=0; c<64; c++ ) {
			free(char_ascii[c]);
		}
		free(char_ascii);
        return 1;
	}
	// here is the selected line
	while(temp < selected_line-1) {
		read(fd1,onechar,1);
		if(onechar[0] == '\n') {
			temp++;
		}
	}
	printf("selected line: %d\n", selected_line);
	printf("the line: \n");
	while (fd1!=-1) {
		if(read(fd1,onechar,1) == 0) 
			break;
		onechar[1] = '\0'; 
		if(onechar[0] != '\n'){
			printf("%s", onechar);
			// here is the line
		}
		else 
			break;
	}
	close(fd1);
	close(fd2);

	free(input_filename);
	free(output_filename);
	for ( c=0; c<64; c++ ) {
		free(char_ascii[c]);
	}
	free(char_ascii);

	return 0;
}