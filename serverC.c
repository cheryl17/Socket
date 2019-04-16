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
#include <math.h>

#define MYPORT "23035" 
#define MAXBUFLEN 10

char link_id[MAXBUFLEN];
char size[MAXBUFLEN];
char length[MAXBUFLEN];
char power[MAXBUFLEN];
char bandwidth[MAXBUFLEN];
char velocity[MAXBUFLEN];
char noise_power[MAXBUFLEN];

double T_trans;
double T_prop;
double endtoend_delay;

int Compute(){
  double capacity;
  double SNR;
  double size_c = atof(size);
  double length_c = atof(length);
  double power_c = atof(power);
  double bandwidth_c = atof(bandwidth);
  double velocity_c = atof(velocity);
  double noise_power_c = atof(noise_power);

 
  SNR = (pow(10,power_c/10))/(pow(10,noise_power_c/10));
  capacity = 1000*bandwidth_c*(log2(1+SNR));
  T_prop = length_c/velocity_c;
  T_trans = size_c/capacity;
  endtoend_delay = T_trans + T_prop;
  printf("The server C finished calculation for link %s.\n", link_id);
}

int main(void)
{
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  //int numbytes;
  struct sockaddr_storage their_addr;
  //char buf[MAXBUFLEN];
  socklen_t addr_len;
 // char s[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC; 
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; 

  if ((rv = getaddrinfo("localhost", MYPORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // 用循环找出全部的结果，并 bind 到首先找到能 bind 的
  for(p = servinfo; p != NULL; p = p->ai_next) {

    if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
      perror("ServerC's listener: socket");
      continue;
    }

   if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
       close(sockfd);
       perror("ServerC's listener: bind");
       continue;
  }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "ServerC's listener: failed to bind socket\n");
    return 2;
  }

  freeaddrinfo(servinfo);
  printf("ServerC is up and running using UDP on port %s.\n", MYPORT);
  

 while(1){
    addr_len = sizeof their_addr;
    char link_id[5];
    recvfrom(sockfd, link_id, sizeof link_id, 0, (struct sockaddr *)&their_addr, &addr_len);
    recvfrom(sockfd, size, sizeof size, 0, (struct sockaddr *)&their_addr, &addr_len);
    recvfrom(sockfd, power, sizeof power , 0, (struct sockaddr *)&their_addr, &addr_len);
    recvfrom(sockfd, bandwidth, sizeof bandwidth , 0, (struct sockaddr *)&their_addr, &addr_len);
    recvfrom(sockfd, length, sizeof length, 0, (struct sockaddr *)&their_addr, &addr_len);
    recvfrom(sockfd, velocity, sizeof velocity, 0, (struct sockaddr *)&their_addr, &addr_len);
    recvfrom(sockfd, noise_power, sizeof noise_power, 0, (struct sockaddr *)&their_addr, &addr_len);

    printf("The Server C received link information of Link %s, file size %s, and signal power  %s\n", link_id, size, power);
    Compute();

    sendto(sockfd, (char *)& T_trans, sizeof T_trans , 0, (struct sockaddr *)&their_addr, addr_len);  
    sendto(sockfd, (char *)& T_prop, sizeof T_prop , 0, (struct sockaddr *)&their_addr, addr_len);  
   sendto(sockfd, (char *)& endtoend_delay, sizeof endtoend_delay, 0, (struct sockaddr *)&their_addr, addr_len);  
  printf("Server C finished sending the output to AWS.\n");
  }
}
