#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
char directory[2048];

void* handle_client(void* arg) {
  int client_fd = *(int*)arg;
  char buffer[BUFFER_SIZE];
  char buffercpy[BUFFER_SIZE];
  if (client_fd < 0) {
    printf("Accept Failed: %s\n", strerror(errno));
  }
  printf("Accepting the connection\n");
  memset(buffer, 0, BUFFER_SIZE);
  read(client_fd, buffer, BUFFER_SIZE);
  strcpy(buffercpy, buffer);
  printf("Message from client : %s \n", buffer);
  /* TOKENIZERS */
  char* first_method = strtok(buffer, "/");
  char* second_str = strtok(NULL, "/");
  /* RESPONSE TYPES */
  char normal_response[35] = "HTTP/1.1 200 OK\r\n\r\n ";
  char notfound[35] = "HTTP/1.1 404 Not Found\r\n\r\n ";
  char echo_string_type[BUFFER_SIZE];
  char error_response[40] = "ERROR RESPONSE FROM SERVER";
  if (first_method && second_str && strcmp(first_method, "GET ") == 0) {
    if (strcmp(second_str, "/") == 0) {
      send(client_fd, normal_response, sizeof(normal_response), 0);
    } else if (strcmp(second_str, "echo") == 0) {
      char* specific_str = strtok(NULL, "/ ");
      sprintf(echo_string_type,
              "HTTP/1.1 200 OK\r\nContent-Type: "
              "text/plain\r\nContent-Length: %lu\r\n\r\n%s",
              strlen(specific_str), specific_str);
      send(client_fd, echo_string_type, BUFFER_SIZE, 0);
    } else if (strcmp(second_str, " HTTP") == 0) {
      send(client_fd, normal_response, 40, 0);
    } else if (strstr(buffercpy, "user-agent")) {
      char* user_agent = strstr(buffercpy, "User-Agent: ");
      user_agent += strlen("User-Agent: ");
      char* eof = strstr(user_agent, "\r\n");
      if (eof) {
        *eof = '\0';
      }
      sprintf(echo_string_type,
              "HTTP/1.1 200 OK\r\nContent-Type: "
              "text/plain\r\nContent-Length: %lu\r\n\r\n%s",
              strlen(user_agent), user_agent);
      send(client_fd, echo_string_type, BUFFER_SIZE, 0);
    } else if (strcmp(second_str, "files") == 0) {
      char* specific_str = strtok(NULL, "/ ");
      if (specific_str == NULL) {
        send(client_fd, notfound, sizeof(notfound), 0);
        return 0;
      }
      strcat(directory, specific_str);
      FILE* fp = fopen(directory, "rb");
      char content[BUFFER_SIZE];
      if (!fp) {
        send(client_fd, notfound, strlen(notfound), 0);
        return 0;
      }
      int bytes_read = fread(content, 1, BUFFER_SIZE, fp);
      char* format =
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: application/octet-stream\r\n"
          "Content-Length: %zu\r\n\r\n%s";
      printf("THER DIRE IS %s \n", directory);
      if (bytes_read > 0) {
        char* response = malloc(BUFFER_SIZE);
        sprintf(response, format, bytes_read, content);
        printf("REPONSE WILL BE (%s)", response);
        send(client_fd, response, strlen(response), 0);
        free(response);
      } else {
        printf("File Dosen't exists or failed to open file");
        send(client_fd, notfound, strlen(notfound), 0);
        return 0;
      }
    } else {
      send(client_fd, notfound, sizeof(notfound), 0);
    }
  } else {
    printf("Response to client failed : %s", strerror(errno));
  }
  return NULL;
}
int main(int argc, char* argv[]) {
  if (argc >= 2 && (strncmp(argv[1], "--directory", 11) == 0)) {
    strcpy(directory, argv[2]);
  }
  printf("THe directory is : %s", directory);
  pthread_t thread_id;
  printf("Logs from the program \n");
  int server_fd, *client_fd;
  struct sockaddr_in client_addr, serv_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  char buffer[BUFFER_SIZE];
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
  serv_addr.sin_port = htons(4221);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("Bind failed: %s\n", strerror(errno));
    return 1;
  }
  if (listen(server_fd, 10) < 0) {
    printf("Listen failed: %s\n", strerror(errno));
    return 1;
  }

  printf("Server is listening on port 4221\n");
  while (1) {
    client_fd = malloc(sizeof(int));
    if ((*client_fd = accept(server_fd, (struct sockaddr*)&client_addr,
                             &client_addr_len)) < 0) {
      printf("Accept failed: %s\n", strerror(errno));
      free(client_fd);
      continue;
    }
    if (pthread_create(&thread_id, NULL, handle_client, client_fd) != 0) {
      printf("Thread creation failed: %s\n", strerror(errno));
      free(client_fd);
    } else {
      pthread_detach(thread_id);
    }
  }
  close(server_fd);
  return 0;
}
