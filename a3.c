#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>

int exit1 = 1;


int main(int arg, char**argv){
	if(mkfifo("RESP_PIPE_27620", 0600) < 0){
		printf("ERROR\n");
		printf("cannot create the response pipe\n");
		return 1;
	}
	int fd1;
	int fd2; 

	fd2 = open("REQ_PIPE_27620", O_RDONLY);
	if((fd2 < 0)){
		printf("ERROR\n");
		printf("cannot open the request pipe\n");
		return 2;
	}
	fd1 = open("RESP_PIPE_27620", O_WRONLY);
	if((fd1 < 0)){
		printf("ERROR\n");
		printf("cannot open the response pipe\n");
		return 3;
	}
	int size = strlen("CONNECT");
	write(fd1, &size, 1);
	write(fd1, "CONNECT", size);
	
	while(exit1 == 1){
		char request;
		read(fd2, &request, sizeof(char));
		char buf[100];
		read(fd2, &buf, request);
		buf[(int)request] = '\0';
		if(strcmp(buf, "PING") == 0){
				int psize = strlen("PING");
				write(fd1, &psize, 1);
				write(fd1, "PING", psize);

				int psize2 = strlen("PONG");
				write(fd1, &psize2, 1);
				write(fd1, "PONG", psize2);
		
				int nbr = 27620;
				write(fd1, &nbr, sizeof(unsigned int));
		}
		if(strcmp(buf, "CREATE_SHM") == 0){
				int mem;
				read(fd2, &mem, sizeof(unsigned int));
				int psize = strlen("CREATE_SHM");
				write(fd1, &psize, 1);
				write(fd1, "CREATE_SHM", psize);

				int shm_fd = shm_open("/LmtHTKl", O_CREAT | O_RDWR, 0664);
				if(shm_fd < 0){
					int err1 = strlen("ERROR");
					write(fd1, &err1, 1);
					write(fd1, "ERROR", err1);
					return 1;
				}
				ftruncate(shm_fd, mem);
				mmap(0, mem, PROT_WRITE, MAP_SHARED, shm_fd, 0);
				int succ = strlen("SUCCESS");
				write(fd1, &succ, 1);
				write(fd1, "SUCCESS", succ);
				
		}
		if(strcmp(buf, "WRITE_TO_SHM") == 0){
				int offset;
				int value;
				read(fd2, &offset, sizeof(unsigned int));
				read(fd2, &value, sizeof(unsigned int));
				int psize = strlen("WRITE_TO_SHM");
				write(fd1, &psize, 1);
				write(fd1, "WRITE_TO_SHM", psize);

				int shm_fd = shm_open("/LmtHTKl", O_RDWR, 0664);
				if(shm_fd < 0){
					int err1 = strlen("ERROR");
					write(fd1, &err1, 1);
					write(fd1, "ERROR", err1);
					return 1;
				}
				if(offset > 3790077){
					int err1 = strlen("ERROR");
					write(fd1, &err1, 1);
					write(fd1, "ERROR", err1);
					return 1;
				}

				if((offset + sizeof(value)) > 3790077){
					int err1 = strlen("ERROR");
					write(fd1, &err1, 1);
					write(fd1, "ERROR", err1);
					return 1;
				}
				ftruncate(shm_fd, 3790077);
				char *pp = (char*) mmap(0, 3790077, PROT_WRITE, MAP_SHARED, shm_fd, 0);
				*(unsigned int *)(pp + offset) = value;
					
				
				int succ = strlen("SUCCESS");
				write(fd1, &succ, 1);
				write(fd1, "SUCCESS", succ);
				
		}
		if(strcmp(buf, "MAP_FILE") == 0){
			char nameLength;
			read(fd2, &nameLength, 1);
			char name[100];
			read(fd2, &name, nameLength);
			name[(int)nameLength] = '\0';

			int psize = strlen("MAP_FILE");
			write(fd1, &psize, 1);
			write(fd1, "MAP_FILE", psize);
			
			int fis;
			fis = open(name, O_RDONLY);
			if (fis < 0){
				int err1 = strlen("ERROR");
				write(fd1, &err1, 1);
				write(fd1, "ERROR", err1);
				return 1;	
			}
			char * data;
			data = (char *) mmap(NULL, size, PROT_READ, MAP_PRIVATE, fis, 0);
			if(data == MAP_FAILED){
				int err1 = strlen("ERROR");
				write(fd1, &err1, 1);
				write(fd1, "ERROR", err1);
				return 1;
			}
			int succ = strlen("SUCCESS");
			write(fd1, &succ, 1);
			write(fd1, "SUCCESS", succ);
		}
		if(strcmp(buf, "READ_FROM_FILE_OFFSET") == 0){
			int psize = strlen("READ_FROM_FILE_OFFSET");
			write(fd1, &psize, 1);
			write(fd1, "READ_FROM_FILE_OFFSET", psize);
			int offset;
			int value;
			read(fd2, &offset, sizeof(unsigned int));
			read(fd2, &value, sizeof(unsigned int));
		}
		if(strcmp(buf, "EXIT") == 0){
				exit1 = 0;			
		}
		
	}
	return 0;
}
