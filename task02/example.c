#include <stdio.h>
#include <pthread.h>

pthread_mutex_t MTX = PTHREAD_MUTEX_INITIALIZER;
void * foo(void*p);

int main(int argc, char ** argv){
	pthread_cond_t c = PTHREAD_MUTEX_INITIALIZER;
	pthread_t tid;

	pthread_create(&tid, 0, foo, &c);
	sleep(2);	//nie uzywac
	pthread_mutex_lock(&MTX);
	printf("WAKE UP!\n");
	pthread_cond_signal(&c);
	pthread_mutex_unlock(&MTX);
	pthread_join(&tid, NULL);
	
	return 0;
}

void * foo(void * a){
	pthread_cond_t * c = (pthread_cond_t*)a;
	pthread_mutex_lock(&MTX);
	pthread_cond_wait(c, &MTX);
	printf("woked up!\n");
	pthread_mutex_unlock(&MTX);
}
