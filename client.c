#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>

#define AWSPORT "25035" // 用户所要连接的 port

int main(int argc, char const *argv[]){
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
 //int numbytes;

  char link_id[10];
  char size[10];
  char power[10];
  strcpy(link_id,argv[1]);
  strcpy(size,argv[2]);
  strcpy(power,argv[3]);
  double T_trans;
  double T_prop;
  double endtoend_delay;
  

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo("localhost", AWSPORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }
     if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
 
    }
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect to AWS.\n");
    return 2;
  }

  freeaddrinfo(servinfo);
  printf("Client is up and running.\n");

  
     send(sockfd, link_id, sizeof link_id, 0);
     send(sockfd, size, sizeof size, 0);
     send(sockfd, power, sizeof power , 0);
     printf("client send  Link_ID = %s\n file size = %s\n signal power = %s\n", link_id, size, power);


 int on = 0;
     recv(sockfd, (char *)& on, sizeof on, 0);
     if (on == 1){
     recv(sockfd, (char *)& T_trans, sizeof T_trans, 0);
     recv(sockfd, (char *)& T_prop, sizeof T_prop, 0);
     recv(sockfd, (char *)& endtoend_delay, sizeof endtoend_delay, 0);
     printf("Link_ID is %s. \n T_trans is %.2f ms. \n T_prop is %.2f ms. \n End-to-end delay is %.2f ms. \n", link_id, T_trans, T_prop, endtoend_delay);
     }
     if (on == -1){
      printf("link %s :Not found.\n", link_id);
     }

}
