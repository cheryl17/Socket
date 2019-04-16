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

#define MYPORT "22035" // 用戶所要连线的 port
#define MAXBUFLEN 100

int i=1;
char *data;
int search(char *link_id){
  char buf[MAXBUFLEN];
  char *row;
  int m=0;
  FILE *file = NULL;
  file = fopen("database_b.csv","r");
  while((row = fgets(buf, sizeof buf, file)) != NULL){
    data = strtok(row, ",");
    if(!strcmp(data, link_id)){
      m = 1;
      break;
    }
    else
      i++;
  }
  fclose(file);
  return m;
}


int main(void)
{
  //UDP in Beej
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  struct sockaddr_storage their_addr;
  socklen_t addr_len;

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC; // 设定 AF_INET 以强制使用 IPv4
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; // 使用我的 IP

  if ((rv = getaddrinfo("localhost", MYPORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // 用循环找出全部的结果，并 bind 到首先找到能 bind 的
  for(p = servinfo; p != NULL; p = p->ai_next) {

    if ((sockfd = socket(p->ai_family, p->ai_socktype,
         p->ai_protocol)) == -1) {
      perror("ServerB listener: socket");
      continue;
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("ServerB listener: bind");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "ServerB listener: failed to bind socket\n");
    return 2;
  }

  freeaddrinfo(servinfo);
  printf("ServerB is up and run on port %s.\n", MYPORT);
  
  
  while(1){
    addr_len = sizeof their_addr;
    char link_id[5];
    recvfrom(sockfd, link_id, sizeof link_id , 0, (struct sockaddr *)&their_addr, &addr_len);
    printf("The server B received input %s\n", link_id);

    int m;
    char *info;
    char t[MAXBUFLEN];
    char *tar;
    m = search(link_id);
    printf("Server B has found %d match.\n", m);
    sendto(sockfd, (char *)& m, sizeof m , 0, (struct sockaddr *)&their_addr, addr_len);
    if (m) {
      char buf[MAXBUFLEN];
      char *row;
      FILE *file = NULL;
      file = fopen("database_b.csv", "r");
      while((row = fgets(buf, sizeof buf, file)) != NULL){
        i = i-1;
        if(!i){
          strcpy(t, row);
          info = strtok(t, "\n");
        }
      }
      fclose(file);
      for(tar = strtok(info, ","); tar != NULL; tar = strtok(NULL, ",")){
      sendto(sockfd, tar, sizeof tar + 1 , 0, (struct sockaddr *)&their_addr, addr_len);  
      }
  }
  i = 1;
  printf("Server B finished sending the output to AWS.\n");
  }
}
