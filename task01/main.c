#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

void*f();
void*e();

int main(int argc, char ** argv){
	int i = 0;
	pthread_t tid[2];
	
	pthread_create(&tid[0],NULL,f,NULL);
	//pthread_create(&tid[1],NULL,f,NULL);
	pthread_join(tid[0],NULL);
	//pthread_join(tid[1],NULL);

	sleep(1);
	printf("main\n");
	return 0;
}

void * f(){
	int p = fork();
	if(p)
		printf("Parent: ");
	else
		printf("Child: ");
	printf("%d\n", p);
}

void * e(){
	execl("/usr/ls","-a", NULL);
}
