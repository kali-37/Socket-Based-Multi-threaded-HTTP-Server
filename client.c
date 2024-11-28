#include <errno.h>
#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include<pthread.h>

#define show_error(str) (fprintf(stderr, "%s: %s\n", str, strerror(errno)))

void* recieve_data(int *socket_desc){
    int  sock =(int)* socket_desc;
    char* server_reply =  malloc(sizeof(char)*20000);
    int recv_size;
    while ((recv_size=recv(sock,server_reply,sizeof(char)*20000,0)) > 0){
        printf("RECIVEV SIZE : %d",recv_size);
        server_reply[recv_size] = '\0';
        printf("Server Reply:\n%s\n",server_reply);
    }
    if (recv_size ==0){
        puts("Server Disconnected");
    }
    else if(recv_size ==-1 ){
        perror("recv failed");
    }
    return NULL;
}

int main(int argc , char *argv[])
{
    pthread_t recieve_thread;
	int socket_desc;
	struct sockaddr_in server;
    char *server_reply= (char*)calloc(1000000,1);
    char* message = malloc(sizeof(char)*30);
	message = "GET / HTTP/1.1\r\n\r\n";
	
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		puts("Could not create socket");
	}
		
	server.sin_addr.s_addr = inet_addr("0.0.0.0");
	server.sin_family = AF_INET;
	server.sin_port = htons( 8000);
    puts(strerror(errno));

	//Connect to remote server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connect error");
		return 1;
	}
	
	puts("Connected");
    if (pthread_create(&recieve_thread,NULL,(void*)&recieve_data,&socket_desc)!=0){
        show_error("Thread connection failed");
    }
	if( send(socket_desc , message , strlen(message) , 0) < 0)
        {
            printf("%s\n",strerror(errno));
            return 1;
        }
    puts("Data send Sucess \n"); 

    pthread_join(recieve_thread,NULL);
	puts("Reply received\n");
	puts(server_reply);
	return 0;
}

