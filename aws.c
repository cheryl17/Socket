#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define UDPPORT "24035"
#define CLIENTPORT "25035"
#define MONITORPORT "26035"
#define PORTA "21035"
#define PORTB "22035"
#define PORTC "23035"
#define BACKLOG 10	//max number of pending connection

char link_id[10]; 
char size[10];
char power[10];
char bandwidth[10];
char length[10]; 
char velocity[10];
char noise[10];
double T_prop;
double T_trans;
double endtoend_delay;

int find_id(char *link_id, char ch){
	int sockUDP;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	const char* sever_port;

	if (ch == 'A')
		sever_port = PORTA;
	if (ch == 'B')
		sever_port = PORTB;


	//set up UDP from Beej
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo("localhost", sever_port ,&hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for (p = servinfo; p != NULL; p->ai_next){
		if ((sockUDP = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("AWS serverA/B UDP: socket");
			continue;
		}
		break;
	}

	if (p == NULL){
		fprintf(stderr, "AWS serverA/B UDP: fail to bind socket.\n");
		return 2;
	}

	//send data using UDP
	sendto(sockUDP, link_id, sizeof link_id, 0, p->ai_addr, p->ai_addrlen);
	printf("The AWS sent link ID=%s to Backend-Server %c using UDP over port %s.\n", link_id, ch, UDPPORT);
	
	addr_len = sizeof their_addr;
	int m;
	recvfrom(sockUDP, (char *)& m, sizeof m, 0, NULL, NULL);
	if (m){
		recvfrom(sockUDP, link_id, sizeof link_id, 0, NULL, NULL);
		recvfrom(sockUDP, bandwidth, sizeof bandwidth, 0, NULL, NULL);
		recvfrom(sockUDP, length, sizeof length, 0, NULL, NULL);
		recvfrom(sockUDP, velocity, sizeof velocity, 0, NULL, NULL);
		recvfrom(sockUDP, noise, sizeof noise, 0, NULL, NULL);
	}
	close(sockUDP);
	return m; 
} 

int set_up_c(){
	//set up UDP connection with serverC from Beej
	int sock_c, rv;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	addr_len = sizeof their_addr;

	memset(&hints, 0, sizeof hints);	//empty the struct
	hints.ai_family = AF_UNSPEC;	//IPv4 and IPv6 both fine
	hints.ai_socktype = SOCK_DGRAM;	//UDP socket

	if((rv = getaddrinfo("localhost", PORTC, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 0;
	}

	//loop for all results and bind the first can use from Beej
	for (p = servinfo; p != NULL; p = p->ai_next){
		if ((sock_c = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("AWS serverC UDP: socket");
			continue;
		}
		break;
	}
	if (p == NULL){
		fprintf(stderr, "AWS serverC UDP: fail to bind socket\n");
		return 0;
	}
	freeaddrinfo(servinfo);

	//send data to server C for computation
	sendto(sock_c, link_id, sizeof link_id, 0, p->ai_addr, p->ai_addrlen);
	sendto(sock_c, size, sizeof size, 0, p->ai_addr, p->ai_addrlen);
	sendto(sock_c, power, sizeof power, 0, p->ai_addr, p->ai_addrlen);
	sendto(sock_c, bandwidth, sizeof bandwidth, 0, p->ai_addr, p->ai_addrlen);
	sendto(sock_c, length, sizeof length, 0, p->ai_addr, p->ai_addrlen);
	sendto(sock_c, velocity, sizeof velocity, 0, p->ai_addr, p->ai_addrlen);
	sendto(sock_c, noise, sizeof noise, 0, p->ai_addr, p->ai_addrlen);
	printf("The AWS sent link ID=%s, size=%s, power=%s and link information to Backend-Server C using UDP over port %s.\n", link_id, size, power, PORTC);

	//recevive computed data from server C
	recvfrom(sock_c, (char *)& T_trans, sizeof T_trans, 0, NULL, NULL);
	recvfrom(sock_c, (char *)& T_prop, sizeof T_prop, 0, NULL, NULL);
	recvfrom(sock_c, (char *)& endtoend_delay, sizeof endtoend_delay, 0, NULL, NULL);
	printf("The AWS received outputs from Backend-Server C using UDP overport %s.\n", UDPPORT);

	//close(sock_c);
}

int main(){
	int sockfd, sockm, new_fd, sock_monitor;	//listen in sock_fd
	struct addrinfo hints, hintsm, *servinfo, *servinfom, *p, *q;
	struct sockaddr_storage their_addr, monitor_addr;
	socklen_t sin_size, mointor_len;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv, rvm;

	//set up TCP from Beej
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo("localhost", CLIENTPORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	//loop for all results and bind the first can use from Beej
	for (p = servinfo; p != NULL; p = p->ai_next){
		if ((sockfd =socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("aws client TCP socket");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
			perror("aws client TCP set sockopt");
			exit(1);
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("aws client TCP bind");
			continue;
		}
		break;
	}
	if (p = NULL){
		fprintf(stderr, "aws client TCP: fail to bind\n");
		return 2;
	}
	freeaddrinfo(servinfo);

	if (listen(sockfd, BACKLOG) == -1){
		perror("aws client TCP listen");
		exit(1);
	}

	//repeat set up TCP for monitor
	memset(&hintsm, 0, sizeof hintsm);
	hintsm.ai_family = AF_UNSPEC;
	hintsm.ai_socktype = SOCK_STREAM;
	hintsm.ai_flags = AI_PASSIVE;

	if ((rvm = getaddrinfo("localhost", MONITORPORT, &hintsm, &servinfom)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rvm));
		return 1;
	}

	for (q = servinfom; q != NULL; q = q->ai_next){
		if ((sockm =socket(q->ai_family, q->ai_socktype, q->ai_protocol)) == -1){
			perror("aws monitor TCP socket");
			continue;
		}
		if (setsockopt(sockm, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
			perror("set monitor TCP sockopt");
			exit(1);
		}
		if (bind(sockm, q->ai_addr, q->ai_addrlen) == -1){
			close(sockm);
			perror("aws monitor TCP bind");
			continue;
		}
		break;
	}

	if (q = NULL){
		fprintf(stderr, "aws monitor TCP: fail to bind\n");
		return 2;
	}
	freeaddrinfo(servinfom);

	if (listen(sockm, BACKLOG) == -1){
		perror("aws monitor TCP listen");
		exit(1);
	}
	printf("The aws is up and running. \n");

	mointor_len = sizeof monitor_addr;
	sock_monitor = accept(sockm, (struct sockaddr*)&monitor_addr, &mointor_len);
	while(1){
	sin_size = sizeof their_addr;
	new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
	if (new_fd == -1){
		perror("accept");
		exit(1);
	}
	/*
	//get the port of client
	struct sockaddr_in addr_client;
	memset(&addr_client, 0,sizeof(addr_client));
	int len = sizeof(addr_client);
	getpeername(new_fd, (struct sockaddr *)&addr_client, (socklen_t *)&len);
	int client_port = addr_client.sin_port;
	*/
	//input from client
	recv(new_fd, link_id, sizeof link_id, 0);
	recv(new_fd, size, sizeof size, 0);
	recv(new_fd, power, sizeof power, 0);
	printf("The AWS received link ID=%s, size=%s, and power=%s from the client using TCP over port %s. \n", link_id, size, power, CLIENTPORT);

	//send received data to monitor
	send(sock_monitor, link_id, sizeof link_id, 0);
	send(sock_monitor, size, sizeof size, 0);
	send(sock_monitor, power, sizeof power, 0);
	printf("The AWS sent link ID=%s, size=%s, and power=%s to the monitor using TCP over port %s.\n", link_id, size, power, MONITORPORT);

	//search in serverA and serverB
	int m1 = find_id(link_id, 'A');
	printf("The AWS recevied %d matches from Backend-Server A using UDP over port %s.\n", m1, UDPPORT);
	int m2 = find_id(link_id, 'B');
	printf("The AWS recevied %d matches from Backend-Server B using UDP over port %s.\n", m2, UDPPORT);
	int match;
	if (!m1 && !m2){
		match = -1;
		send(new_fd, (char *)&match, sizeof match, 0);
		send(sock_monitor, (char *)&match, sizeof match, 0);
		printf("The AWS sent NO MATCH to the monitor and the client using TCP over ports %s and %s, respectively.\n", MONITORPORT, CLIENTPORT);
	}
	else {
		match = 1;
		set_up_c();
		//send end-to-end delay to client
		send(new_fd, (char *)&match, sizeof match, 0);
		send(sock_monitor, (char *)&match, sizeof match, 0);
		send(new_fd, (char *)&endtoend_delay, sizeof endtoend_delay, 0);
		printf("The AWS send delay=%f ms to the client using TCP over port %s.\n", endtoend_delay, CLIENTPORT);

		//send all results to monitor
		send(sock_monitor, (char *)&T_trans, sizeof T_trans, 0);
		send(sock_monitor, (char *)&T_prop, sizeof T_prop, 0);
		send(sock_monitor, (char *)&endtoend_delay, sizeof endtoend_delay, 0);
		printf("The AWS send detailed results to the monitor using TCP over port %s.\n", MONITORPORT);
	}
	close(new_fd);
}
}