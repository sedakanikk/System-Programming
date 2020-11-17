#include <stdio.h> 
#include <stdlib.h> 
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int translate_into_hex(char arr[32], int* arr_ascii) {
	int j;
	for(j=0; j<32; j++) {
		int i = arr[j];
		arr_ascii[j] = i;
	}
	return 1;
}
int power(int number, int pow) {
	int res=1;
	for(int i=0; i<pow; i++) {
		res *= number;
	}
	return res;
}

int main(int argc, char* argv[]) {
	int opt, base = 10, sleep_time=0, time_temp=0;
	char *input_filename, *output_filename;
	unsigned char onechar[2];
	int* arr_hex;
	arr_hex = (int*) malloc(32 * sizeof(int));
	char** char_ascii;
	char_ascii = (char**) malloc(64 * sizeof(char));
	int c;
	for ( c=0; c<64; c++ )
	{
		char_ascii[c] = (char*) malloc(3 * sizeof(char));
	}
	char** temp;
	temp = (char**) malloc(4 * sizeof(char));
	for ( c=0; c<4; c++ )
	{
		temp[c] = (char*) malloc(3 * sizeof(char));
	}
	// ./programA -i inputPathA -o outputPathA -t time
	while((opt = getopt(argc, argv, "iot")) != -1)  
	{  
		switch(opt)  
		{  
			case 'i':  
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
	// to calculate size of buffer
	struct stat st;
	stat(input_filename, &st);
	int size = st.st_size;
	unsigned char buffer[size];
	// if file can not open for any reason
	if (fd1 == -1 || fd2 == -1) {
		free(input_filename);
		free(output_filename);
		free(arr_hex);

		for ( c=0; c<64; c++ ) {
			free(char_ascii[c]);
		}
		free(char_ascii);
		for ( c=0; c<4; c++ ) {
			free(temp[c]);
		}
		free(temp);
        return 1;
	}
    // to write on the function
	temp[1] = " +i";
	temp[2] = ",";
	temp[3] = "\n";

	//checks if file has smaller than 32 element
	int flag=0, i = 0; 
	while (fd1!=-1) {
		if(read(fd1,onechar,1) == 0) 
			break;
		onechar[1] = '\0'; 
		// if there is no new line, write on the file something 
		if(onechar[0] != '\n') {
			buffer[i] = onechar[0];
			// if the number of read bytes is 32, then translate all of them and write on file 
			if(i == 31) {
				// buffer is created and send it to use in function
				translate_into_hex(buffer, &arr_hex[0]);
				int gg;
				for(gg = 0; gg<33; gg++) {
					static char buf[4] = {0};
					// if need to be newline 
					if(gg==32) {
						flag = 1;
						int t = write(fd2, temp[3], 1);
						break;
					}
					int val=arr_hex[gg];
					int index_itoa = 3;
					// except newline, converts all the bytes into the ASCII and writes on file
					for(; val && index_itoa ; --index_itoa, val /= base) {
						buf[index_itoa] = "0123456789abcdef"[val % base];
					}
					if(buf[1]==0 || buf[1]==1 || buf[1]==2 || buf[1]==3 || buf[1]==4 || buf[1]==5 || buf[1]==6 || buf[1]==7 || buf[1]==8 || buf[1]==9) {
						temp[0] = &buf[2];
						flag = 1;
						int t = write(fd2, temp[0], 2);
					}
					else {
						temp[0] = &buf[1];
						flag = 1;
						int t = write(fd2, temp[0], 3);
					}
					if((gg%2)==0) {
						flag = 1;
						int t = write(fd2, temp[1], 3);
					}
					else {
						flag = 1;
						int t = write(fd2, temp[2], 1);
						sleep(sleep_time/1000);
					}
				}
				i=-1;
			}
			i++;
		}		
	}
	int j;
	if(i != -1) {
		for(j=i; j<32; j++) {
			buffer[j] = '\0';
		}
	}
	// close files
	close(fd1);
	close(fd2);


	

	free(input_filename);
	free(output_filename);
	free(arr_hex);

	for ( c=0; c<64; c++ ) {
		free(char_ascii[c]);
	}
	free(char_ascii);
	for ( c=0; c<4; c++ ) {
		free(temp[c]);
	}
	free(temp);
	return 0;
}