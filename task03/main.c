#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAX_USERS 32

struct user{
	int usr_fd;
	char * usr_name;
};

int epoll_fd = -1;
int srv_fd = -1;
int exitFlag = 0;
int port;
int clientCount;
socklen_t cli_addr_len;
int cli_fd;

struct epoll_event e;
struct sockaddr_in cli_addr;
struct sockaddr_in srv_addr;
struct user * users[MAX_USERS];

void signalHandler();
int handleClient(struct epoll_event*);
int handleServer();
int initServer();

int handleAckNack(int,char);
void handleMessage(int, char*);
int handleLogIn(int, char*);
int addUser(int, char*);
int rmUser(int, char*);
int findUserByFd(int);

int main(int argc, const char *argv[])
{
    int i = 0;
    struct epoll_event es[clientCount];


	memset(users, 0, MAX_USERS * sizeof(struct user*));
	struct sigaction sig;
	memset(&sig, '\0', sizeof(sig));
	sig.sa_handler = &signalHandler;
	if(sigaction(SIGINT, &sig, NULL) < 0){
		printf("sigerror");
		return 1;
	}

	if(argv[1] && argv[2]){
		port = atoi(argv[1]);
		clientCount = atoi(argv[2]);
	}
	else{
		printf("invalid args\n");
		return 1;
	}

	initServer();

    while(!exitFlag) {
        i = epoll_wait(epoll_fd, es, 2, -1);
        if (i < 0) {
            printf("Cannot wait for events\n");
            close(epoll_fd);
            close(srv_fd);
            return 1;
        }
        for (--i; i > -1; --i) {
            if (es[i].data.fd == srv_fd)
				handleServer();
			else
				handleClient(&es[i]);
        }
	}

	close(epoll_fd);
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, e.data.fd, &e);
	close(srv_fd);
	return 0;
}

int initServer(){
    memset(&srv_addr, 0, sizeof(srv_addr));
    memset(&e, 0, sizeof(e));

    srv_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
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
	return 0;
}

int handleClient(struct epoll_event * e){
	size_t len = 0;
	char * buff = 0;
	if (e->events & EPOLLIN) {
		if(e->events & EPOLLRDHUP){
			rmUserByFd(e->data.fd);
			return 1;
		}
		read(e->data.fd, &len, sizeof(size_t));
		buff = malloc(len*sizeof(char)+1);
		read(e->data.fd, buff, len);
		buff[len+1] = 0;
        if (len > 0) {
			handleMessage(e->data.fd, buff);
        }
		else{
			rmUserByFd(e->data.fd);
		}
        e->data.fd = -1;
	}

	return 0;

}

int handleServer(){
	cli_fd = -1;
	cli_fd = accept(srv_fd, (struct sockaddr*) &cli_addr, &cli_addr_len);
	memset(&cli_addr, 0, sizeof(cli_addr));
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

void handleMessage(int fd, char * msg){
	char * nick;
	if(msg[1] == '.'){
		switch(msg[0]){
			case '1':
				break;
			case '2':
				nick = malloc(sizeof(msg)-2);
				strcpy(nick, msg+2);
				handleLogIn(fd, msg+2);
				free(nick);
				break;
			case '6':
				handleUserList(fd);
				break;
			default:
				handleAckNack(fd,'0');
				break;
		}
	}
	else
		handleAckNack(fd,'0');
}

int handleAckNack(int fd, char msg){
	char * tempMsg;
	size_t tempLen;
	if(msg == '0')
		tempMsg = "2.nick - login, 6. - user list";
	else if(msg == '1'){
		tempMsg = "Nie mozna Cie Zalogowac, podany nick juz istnieje";
	}
	else if(msg == '2'){
		tempMsg = "Zalogowano";
	}
	else if(msg == '3')
		return 1;
	else if(msg == '4')
		tempMsg = "Jestes juz zalogowany";


	tempLen = strlen(tempMsg);
	write(fd, &tempLen, sizeof(size_t));
	write(fd, tempMsg, tempLen);
	return 0;
}

int handleLogIn(int fd, char * msg){
	if(addUser(fd, msg) == 0)
		handleAckNack(fd,'2');
	else if(addUser(fd,msg) == 1)
		handleAckNack(fd, '1');
	else
		handleAckNack(fd, '4');

}

int handleUserList(int fd){
	int i = 0;
	char * userNames = 0;
	size_t userNamesLen = 0;


	while(i<MAX_USERS){
		if(users[i] != NULL){
			userNamesLen += strlen(users[i]->usr_name)+1;
		}
		i++;
	}
	i = 0;
	userNames = malloc( (userNamesLen+1) * sizeof(char) );
	while(i<MAX_USERS){
		if(users[i] != NULL){
			sprintf(userNames,"%s %s",userNames, users[i]->usr_name);
		}
		i++;
	}
	userNames[userNamesLen+1] = 0;
	write(fd, &userNamesLen, sizeof(size_t));
	write(fd, userNames, userNamesLen);
	return 0;
}

int addUser(int fd, char * name){
	int i = 0;
	if(findUserByFd(fd) == 1) return -1;
	if(findUserByName(name) == 1) return 1;
	while(i<MAX_USERS){
		if(users[i] == NULL){
			users[i] = malloc(sizeof(struct user));
			users[i]->usr_name = malloc( (strlen(name)+1)  * sizeof(char) );
			strcpy( users[i]->usr_name, name);
			users[i]->usr_fd = fd;
			printf("%s logged in\n",name);
			return 0;
		}
		i++;
	}
	return 1;
}

int rmUser(int fd, char * name){
	int i = 0;
	while(i<MAX_USERS){
		if(strcmp(users[i]->usr_name,name)==0){
			free(users[i]->usr_name);
			free(users[i]);
			users[i] = NULL;
			return 0;
		}
	}
	return 1;
}

int rmUserByFd(int fd){
	int i = 0;
	while(i<MAX_USERS){
		if(users[i]->usr_fd == fd){
			printf("%s disconnected\n",users[i]->usr_name);
			free(users[i]->usr_name);
			free(users[i]);

			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &e);
			close(fd);
			return 0;
		}
	}
	return 1;
}

void clearUsers(){
	int i = 0;
	while(i<MAX_USERS){
		if(users[i]){
			free(users[i]->usr_name);
			free(users[i]);
		}
		i++;
	}
	printf("\nDeleted userlist\n");
}

int findUserByFd(int fd){
	int i = 0;
	while(i<MAX_USERS){
		if(users[i] != NULL)
			if(users[i]->usr_fd == fd)
				return 1;
		i++;
	}
	return 0;
}

int findUserByName(char * name){
	int i = 0;
	while(i<MAX_USERS){
		if(users[i] != NULL)
			if(strcmp(users[i]->usr_name,name)==0)
				return 1;
		i++;
	}
	return 0;
}

void signalHandler(){
	clearUsers();
	exitFlag = 1;
}
