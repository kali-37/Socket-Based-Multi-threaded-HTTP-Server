#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>

#define PORT 4221
#define BUFFER_SIZE 1024


void* thread_request(int client_fd){

}

char *str_to_index(int from, int to, char *str) {
  while (from > 0) {
    str++;
    from--;
  }
  char *return_str = malloc(sizeof(char) * (to - from));
  while (to >= 0) {
    *return_str = *str;
    return_str++;
    to--;
  }
  return return_str;
}


int main() {
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
  pthread_t *client_thread;

  printf("Logs from the program \n");
  int server_fd, client_fd;
  struct sockaddr_in client_addr, serv_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  char buffer[BUFFER_SIZE];
  char buffercpy[BUFFER_SIZE];

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    printf("Socket creation failed: %s...\n", strerror(errno));
    return 1;
  }

  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    printf("SO_REUSEADDR failed: %s \n", strerror(errno));
    return 1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(4221), serv_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
    printf("Bind failed: %s \n", strerror(errno));
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) < 0) {
    printf("Listen failed: %s \n", strerror(errno));
    return 1;
  }
  printf("Waiting for a client to connect...\n");
  while (1) {
    printf("CLIENT...\n");
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0) {
      printf("Accept Failed: %s\n", strerror(errno));
      continue;
    }
    printf("accepting conn");
    client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0) {
      printf("Accept Failed: %s\n", strerror(errno));
      continue;
    }
    memset(buffer, 0, BUFFER_SIZE);
    read(client_fd, buffer, BUFFER_SIZE);
    strcpy(buffercpy, buffer);

    printf("Message from client : %s \n", buffer);

    /* TOKENIZERS */
    char *first_method = strtok(buffer, "/");
    char *second_str = strtok(NULL, "/");

    /* RESPONSE TYPES */
    char normal_response[35] = "HTTP/1.1 200 OK\r\n\r\n ";
    char notfound[35] = "HTTP/1.1 404 Not Found\r\n\r\n ";
    char echo_string_type[BUFFER_SIZE];
    char error_response[40] = "ERROR RESPONSE FROM SERVER";

    if (first_method && second_str && strcmp(first_method, "GET ") == 0) {
      if (strcmp(second_str, "/") == 0) {
        send(client_fd, normal_response, sizeof(normal_response), 0);
      } else if (strcmp(second_str, "echo") == 0) {
        char *specific_str = strtok(NULL, "/ ");
        sprintf(echo_string_type,
                "HTTP/1.1 200 OK\r\nContent-Type: "
                "text/plain\r\nContent-Length: %lu\r\n\r\n%s",
                strlen(specific_str), specific_str);
        send(client_fd, echo_string_type, BUFFER_SIZE, 0);
      } else if (strcmp(second_str, " HTTP") == 0) {
        send(client_fd, normal_response, 40, 0);
      } else if (strstr(buffercpy, "user-agent")) {
        char *user_agent = strstr(buffercpy, "User-Agent: ");
        if ((!user_agent)) {
          continue;
        }
        user_agent += strlen("User-Agent: ");
        char *eof = strstr(user_agent, "\r\n");
        if (eof) {
          *eof = '\0';
        }
        sprintf(echo_string_type,
                "HTTP/1.1 200 OK\r\nContent-Type: "
                "text/plain\r\nContent-Length: %lu\r\n\r\n%s",
                strlen(user_agent), user_agent);
        send(client_fd, echo_string_type, BUFFER_SIZE, 0);
      } else {
        send(client_fd, notfound, sizeof(notfound), 0);
      }

    } else {
      printf("Response to client failed : %s", strerror(errno));
    }
  }
  return 0;
}