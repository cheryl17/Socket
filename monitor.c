#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define AWSPORT "26035" // 用户所要连接的 port

int main(){
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  
  char link_id[10];
  char size[10];
  char power[10];
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

  // 用循环找出全部的结果，并产生一个 socket
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
        p->ai_protocol)) == -1) {
      perror("Monitor: socket");
      continue;
    }
    if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
      close(sockfd);
      perror("Monitor: connected");
    }
    break;
  }

  if (p == NULL) {
    fprintf(stderr, " monitor failed to connect to AWS.\n");
    return 2;
  }

    freeaddrinfo(servinfo);
    printf("Monitor is up and running.\n");

  while(1) {
     recv(sockfd, link_id, sizeof link_id, 0);
     recv(sockfd, size, sizeof size, 0);
     int socket_data =  recv(sockfd, power, sizeof power , 0);
     if(socket_data)
     printf("Monitor received from AWS:\nLink_ID = %s\n file size = %s\n signal power = %s\n", link_id, size, power);

     int on = 0;
     recv(sockfd, (char *)& on, sizeof on, 0);
     if (on ==1){
     recv(sockfd, (char *)& T_trans, sizeof T_trans, 0);
     recv(sockfd, (char *)& T_prop, sizeof T_prop, 0);
     recv(sockfd, (char *)& endtoend_delay, sizeof endtoend_delay, 0);
     printf("the result of link %s. \n T_trans is %.2f ms. \n T_prop is %.2f ms. \n End-to-end delay is %.2f ms. \n", link_id, T_trans, T_prop, endtoend_delay);
     }
     if (on ==-1){
      printf("link %s Not found.\n", link_id);
     }
     if(!sockfd)
      break;
  } 
  close(sockfd);
  return 0;
}
