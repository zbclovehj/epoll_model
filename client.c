#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>

#include<stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define SERVER_PORT 33333
#define MAXLEN 1024

void client_handle(int sock);
int main(int argc,char* argv[]){
int i;
for ( i = 1; i < argc; ++i) {
        printf("input args %d: %s\n", i, argv[i]);
    }
struct sockaddr_in seraddr;
int server_port = SERVER_PORT;
if(2==argc){
server_port = atoi(argv[1]);
}
int sock = socket(AF_INET,SOCK_STREAM,0);
bzero(&seraddr,sizeof(seraddr));
seraddr.sin_family = AF_INET;
seraddr.sin_port = htons(server_port);
inet_pton(AF_INET, "127.0.0.1", &seraddr.sin_addr);
connect(sock,(struct sockaddr*)&seraddr,sizeof(seraddr));
client_handle(sock);
}

void client_handle(int sock){
char sendbuf[MAXLEN],recvbuf[MAXLEN];
bzero(sendbuf,MAXLEN);
bzero(recvbuf,MAXLEN);
int n = 0;
while(1){
if(NULL == fgets(sendbuf,MAXLEN,stdin)){
break;
}
if('#'==sendbuf[0]){
break;
}
write(sock,sendbuf,strlen(sendbuf));
n = read(sock,recvbuf,MAXLEN);
if(n==0){
break;
}
write(STDOUT_FILENO,recvbuf,n);
}
close(sock);


}
