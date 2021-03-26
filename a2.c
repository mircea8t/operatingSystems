#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "a2_helper.h"
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <fcntl.h>

#define SNAME1 "/mysem"
#define SNAME2 "/mysema"

sem_t sem3;
sem_t sem5Start;
sem_t sem5End;


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t lockU = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condU = PTHREAD_COND_INITIALIZER;

sem_t *sem59Start;
sem_t *sem59End;


int8_t countThread = 0;
int8_t endTwo = 0;
int ht_no = 0;

void P(sem_t* sem){
	sem_wait(sem);
}

void V(sem_t* sem){
	sem_post(sem);
}

void PP(sem_t** sem){
	sem_wait(*sem);
}

void VV(sem_t** sem){
	sem_post(*sem);
}

//create threads only after the fork
typedef struct{
	int proc;
	int th;
} th_struct;

void* func5(void* arg){
	th_struct* s = (th_struct*) arg;
	if(s->th == 3){
		info(BEGIN, s->proc, s->th);
		V(&sem5Start);
		P(&sem5End);
		info(END, s->proc, s->th);
		
	}else{
		if(s->th == 2){
			P(&sem5Start);
			info(BEGIN, s->proc, s->th);
			info(END, s->proc, s->th);
			V(&sem5End);
		}else{
			if(s->th == 4){
				PP(&sem59Start);
				info(BEGIN, s->proc, s->th);
				info(END, s->proc, s->th);
				VV(&sem59End);
			}else{
				info(BEGIN, s->proc, s->th);
				info(END, s->proc, s->th);
			}
		}	
	}			
	return NULL;
}

void* func3(void* arg){
	th_struct* s = (th_struct*) arg;
	P(&sem3);
	info(BEGIN, s->proc, s->th);
	if(s->th == 12){
		pthread_mutex_lock(&lockU);
		while (ht_no > 5){
			pthread_cond_wait(&condU, &lockU);
		}
		pthread_mutex_unlock(&lockU);
		ht_no++;
	}
	
	pthread_mutex_lock(&lockU);
	ht_no--;
	pthread_cond_signal(&condU);
	info(END, s->proc, s->th);
	pthread_mutex_unlock(&lockU);
	V(&sem3);
	return NULL;
}

void* func9(void* arg){
	th_struct* s = (th_struct*) arg;
	if(s->th == 1){
		info(BEGIN, s->proc, s->th);
		info(END, s->proc, s->th);
		VV(&sem59Start);
	}else{
		if(s->th == 2){
			PP(&sem59End);
			info(BEGIN, s->proc, s->th);
			info(END, s->proc, s->th);
		}else{
			info(BEGIN, s->proc, s->th);
			info(END, s->proc, s->th);
		}
	}
	return NULL;
}


int main(){
    init();

    info(BEGIN, 1, 0);
    int childPid = fork();
	switch (childPid){
	case -1:
		exit(1);
	case 0://P2
		info(BEGIN, 2, 0);
		childPid = fork();
		switch(childPid){
		case -1:
			exit(2);
		case 0://P5
			info(BEGIN, 5, 0);
			pthread_t threads[4];
			th_struct param[4];
			sem59Start = sem_open(SNAME1, O_CREAT, 0644, 0);
			sem59End = sem_open(SNAME2, O_CREAT, 0644, 0);
			sem_init(&sem5Start, 1, 0);
			sem_init(&sem5End, 1, 0);
			for(int i=0; i < 4; ++i){
				param[i].proc = 5;
				param[i].th = i+1;
				pthread_create(&threads[i], NULL, func5, (void*)&param[i]);
			}
			for(int i=0; i<4; i++){
				pthread_join(threads[i], NULL);
			}
			sem_destroy(&sem5End);
			sem_destroy(&sem5Start);
			info(END, 5, 0);
			break;		
		default://P2
			wait(NULL);
			info(END, 2, 0);
			break;
			}
		break;
	default:
		childPid = fork();
		switch(childPid){
		case -1:
			exit(3);
		case 0://P3
			info(BEGIN, 3, 0);
			childPid = fork();
			switch(childPid){
			case -1:
				exit(3);
			case 0://P6
				info(BEGIN, 6, 0);
				childPid = fork();
				switch(childPid){
				case -1:
					exit(3);
				case 0://P7
					info(BEGIN, 7, 0);
					info(END, 7, 0);
					break;
				default://P6
					wait(NULL);
					info(END, 6, 0);
				}
				break;
			default://P3
				wait(NULL);
				pthread_t threads[49];
				th_struct param[49];
				sem_init(&sem3, 1, 5);
				for(int i=0; i < 49; ++i){
					param[i].proc = 3;
					param[i].th = i+1;
					pthread_create(&threads[i], NULL, func3, (void*)&param[i]);
				}
				for(int i=0; i<49; i++){
					pthread_join(threads[i], NULL);
				}
				sem_destroy(&sem3);
				info(END, 3, 0);
				break;
			}	
			break;
		default:
			childPid = fork();
			switch(childPid){
			case -1:
				exit(4);
			case 0://P4
				info(BEGIN, 4, 0);
				childPid = fork();
				switch(childPid){
				case -1:
					exit(4);
				case 0://P9
					info(BEGIN, 9, 0);
					pthread_t threads[4];
					th_struct param[4];
					sem59Start = sem_open(SNAME1, 0);
					sem59End = sem_open(SNAME2, 0);
					for(int i=0; i < 4; ++i){
						param[i].proc = 9;
						param[i].th = i+1;
					pthread_create(&threads[i], NULL, func9, (void*)&param[i]);
					}
					for(int i=0; i<4; i++){
					pthread_join(threads[i], NULL);
					}
					info(END, 9, 0);
					break;
				default://P4
					wait(NULL);
					info(END, 4, 0);
					break;
				}			
				break;
			default://P1
				childPid = fork();
				switch(childPid){
				case -1:
					exit(6);
				case 0://P8
					info(BEGIN, 8, 0);
					info(END, 8, 0);
					break;
				default://P1
					wait(NULL);
					wait(NULL);
					wait(NULL);
					wait(NULL);
					info(END, 1, 0);
					break;
				}
				break;
			}
			break;
		}
		break;
	}
    return 0;
}
