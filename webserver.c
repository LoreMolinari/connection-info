#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "ipaddress.h"

#define PORT 9090
#define LISTEN 5
#define MAXLEN 255

int main(int argc, char **argv)
{

  struct sockaddr_in bind_ip_port;
  int bind_ip_port_length = sizeof(bind_ip_port);

  int serverdescriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (serverdescriptor < 0) die("socket() error.\n");
  printf("socket() ok.\n");

  bind_ip_port.sin_family = AF_INET; 
  bind_ip_port.sin_addr.s_addr = inet_addr("127.0.0.1");
  bind_ip_port.sin_port = htons(PORT);

  if(bind(serverdescriptor, (struct sockaddr *) &bind_ip_port, bind_ip_port_length) < 0) die("bind() error.\n");
  printf("bind() ok.\n");

  if(listen(serverdescriptor, LISTEN) < 0) die("listen() error.\n");
  printf("listen ok.\n");

  while (1){

    struct sockaddr_in client_addr;
    socklen_t clientaddr_len;

    int clientdescriptor = accept(serverdescriptor, (struct sockaddr *)&client_addr, &clientaddr_len);
    if (clientdescriptor < 0) die("accept() error.\n");
    printf("accept() ok.\n");
    printf("---------- new client connected ----------\n");

    pid_t pid = fork();

    if (pid < 0) {
      die("fork() error.\n"); 
    } else if (pid > 0) {
      close(clientdescriptor); 
    } else if (pid == 0) {
      close(serverdescriptor);
      handleClient(clientdescriptor);
      printf("---------- client disconnected ----------\n");
      close(clientdescriptor);
      exit(0);
    }

  }

  close(serverdescriptor);
  return 0;
}


void handleClient(int clientdescriptor) {

    FILE* f;

    f = fdopen(clientdescriptor, "w+");
    if (f == NULL) die("fdopen() error.\n");
    printf("fdopen() ok.\n");

    sendFirstResponse(f, get_ip_address(f), "test");

    fclose(f);
    // stdout needs to be flushed in order for heroku to read the logs
    fflush(stdout);
    printf("fflush()\n");

}

void sendFirstResponse(FILE *f, char *key, char *value)
{

  const char htmlResponse[] = "\r\n <html>\r\n <head>\r\n <title>Protocols calculations</title>\r\n </head>\r\n <body>\r\n"
  "<h1>Scegliere il metodo da eseguire: </h1>\r\n <form action='' method='get'> <input type='submit' id='t' name='choice' value='Throughput'>\r\n"
  "<input type='submit' id='i' name='choice' value='IdleRQ'>\r\n <input type='submit' id='w' name='choice' value='AdvertisedWindow'>\r\n"
  "<input type='submit' id='TT' name='choice' value='Timeout'></form>\r\n"
  "</body>\r\n </html>\r\n\r\n";

  fprintf(f, "HTTP/1.1 200 OK\r\n");
  fprintf(f, "Content-Type: text/html\r\n");
  fprintf(f, htmlResponse);
}

/*void methodSelection(clientdescriptor){

  char* selection;
  selection = get_selection();
  
  if (strcmp(selection, "T") == 0) {
    throughput(clientdescriptor);
  } else if (strcmp(selection, "I") == 0) {
    idleRQ(clientdescriptor);
  } else if (strcmp(selection, "W") == 0) {
    window(clientdescriptor);
  } else if (strcmp(selection, "TT") == 0) {
    timeout(clientdescriptor);
  } else {
    invalidmethod();
  }

}*/

//funzione usata in caso di erroe per terminare il programma e stampare un messaggio di errore
void die(char *error) {
    
    fprintf(stderr, "%s.\n", error);
    exit(1);

}