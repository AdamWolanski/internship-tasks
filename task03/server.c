#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

int srv_fd = -1;
int epoll_fd = -1;
struct epoll_event e;

void handler();
void handleClient(struct epoll_event*);
int handleServer();
int initServer(int,int);
static int exitFlag = 0;

int main(int argc, const char *argv[])
{
	fcntl(epoll_fd, F_SETFL, fcntl(epoll_fd, F_GETFL, 0) | O_NONBLOCK);
	int i = 0, port, clientCount;
    char * buff = 0;
    size_t msg_len = 0;

	if(argv[1] && argv[2]){
		port = atoi(argv[1]);
		clientCount = atoi(argv[2]);
	}
	else{
		printf("args\n");
		return 1;
	}
    struct epoll_event es[clientCount];
	struct sigaction sig;
    memset(&e, 0, sizeof(e));
	memset(&sig, '\0', sizeof(sig));

	sig.sa_handler = &handler;
	if(sigaction(SIGINT, &sig, NULL) < 0){
		printf("sigerror");
		return 1;
	}
	if(initServer(port, clientCount) != 0)
		return 1;

    for(;;) {
		i = epoll_wait(epoll_fd, es, clientCount, -1);
        if (i < 0) {
            printf("Cannot wait for events\n");
            close(epoll_fd);
            close(srv_fd);
            return 1;
        }

		/*if(exitFlag){
			close(cli_fd);
			close(epoll_fd);
			close(srv_fd);
			return 0;
		}*/

        for (--i; i > -1; --i) {
            if (es[i].data.fd == srv_fd)
                handleServer();
			else
				handleClient(&es[i]);

        }
    }

	return 0;
}

int initServer(int port, int clientCount){
	struct sockaddr_in srv_addr;
	memset(&srv_addr, 0, sizeof(srv_addr));

	srv_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    fcntl(srv_fd, F_SETFL, fcntl(srv_fd, F_GETFL, 0) | O_NONBLOCK);
	if (srv_fd < 0) {
        printf("Cannot create socket\n");
        return 1;
    }

	srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(port);
    if (bind(srv_fd, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0) {
        printf("Cannot bind socket\n");
        close(srv_fd);
        return 1;
    }

    if (listen(srv_fd, clientCount-1) < 0) {
        printf("Cannot listen\n");
        close(srv_fd);
        return 1;
    }

    epoll_fd = epoll_create(clientCount);
    if (epoll_fd < 0) {
        printf("Cannot create epoll\n");
        close(srv_fd);
        return 1;
    }

    e.events = EPOLLIN;
    e.data.fd = srv_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, srv_fd, &e) < 0) {
        printf("Cannot add server socket to epoll\n");
        close(epoll_fd);
        close(srv_fd);
        return 1;
    }

}

int handleServer(){
	struct sockaddr_in cli_addr;
	socklen_t cli_addr_len;
	memset(&cli_addr, 0, sizeof(cli_addr));
	int cli_fd = -1;
	cli_fd = accept(srv_fd, (struct sockaddr*) &cli_addr, &cli_addr_len); 
	fcntl(cli_fd, F_SETFL, fcntl(cli_fd, F_GETFL, 0) | O_NONBLOCK);
	if (cli_fd < 0) {
		printf("Cannot accept client\n");
        close(epoll_fd);
        close(srv_fd);
        return 1;
	}

	e.data.fd = cli_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cli_fd, &e) < 0) {
		printf("Cannot accept client\n");
		close(epoll_fd);
		close(srv_fd);
        return 1;
	}
	return 0;
}

void handleClient(struct epoll_event * e){
	size_t msg_len = 0;
	char * buff = 0;
	if (e->events & EPOLLIN) {
		read(e->data.fd, &msg_len, sizeof(size_t));
		buff = malloc(msg_len * sizeof(char));
		read(e->data.fd, buff, msg_len);
        if (msg_len > 0) {
			write(e->data.fd, &msg_len, sizeof(size_t));
			write(e->data.fd, buff, msg_len);
        }
	}
}

void handler(){
	exitFlag = 1;
}

void parseMessage(char * buff){
	if(buff[1] == '.'){
		if(buff[0] == '1'){

		}
		if(buff[0] == '2'){

		}
	}
}

//epoll_clt(epoll_fd,EPOLL_CTL_DEL, . , . )
