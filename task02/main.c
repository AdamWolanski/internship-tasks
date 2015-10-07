#include <stdio.h>
#include <pthread.h>

pthread_mutex_t MTX = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;

//test
//test2

int ping = 1;
int count = 0;
int arg;
void*pingPong(void*);

int main(int argc, char ** argv){	
	pthread_t thread[2];
	if(argv[1]) 
		arg = atoi(argv[1]);
	else if(!argv[1])
		return -1;

	pthread_create(&thread[0],0, pingPong, &c);
	pthread_create(&thread[1],0, pingPong, &c);	
		
	pthread_join(thread[0], NULL);	
	pthread_join(thread[1], NULL);	
}


void * pingPong(void * val){
	pthread_cond_t * mutex = (pthread_cond_t*)val;
	while(count<arg){
		pthread_mutex_lock(&MTX);
		if(ping == 1){ 
			printf("Ping\n");
			ping = 0;
		}
		else{
			printf("\tPong\n");
			ping = 1;		
		}
		count++;
	
		pthread_cond_signal(&c);
		pthread_cond_wait(mutex, &MTX);
		pthread_mutex_unlock(&MTX);
	}
	pthread_cond_signal(&c);
	pthread_exit(0);
}
