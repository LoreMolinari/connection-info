#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "getRequest.h"

#define PORT 9091
#define LISTEN 5
#define MAXLEN 255
#define MAX_LINE_LENGTH (1024)

char* scanString(char* s);

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

    sendFirstResponse(f, getRequest(f), "test");

    fclose(f);
    // stdout needs to be flushed in order for heroku to read the logs
    fflush(stdout);
    printf("fflush()\n");

}

void sendFirstResponse(FILE *f, char* key, char *value)
{ 
  const char htmlResponse[] = "\r\n <html>\r\n <head>\r\n <title>Protocols calculations</title>\r\n </head>\r\n <body>\r\n"
  "<h1>Scegliere il metodo da eseguire: </h1>\r\n <form action='' method='get'> <input type='submit' id='t' name='choice' value='Throughput'>\r\n"
  "<input type='submit' id='i' name='choice' value='IdleRQ'>\r\n <input type='submit' id='w' name='choice' value='AdvertisedWindow'>\r\n"
  "<input type='submit' id='TT' name='choice' value='Timeout'></form>\r\n"
  "</body>\r\n </html>\r\n\r\n";

  fprintf(f, "HTTP/1.1 200 OK\r\n");
  fprintf(f, "Content-Type: text/html\r\n");
  fprintf(f, htmlResponse);

  methodSelection(f, scanString(key));

}

char* scanString(char* s){
  int i=0;
  char* choice = malloc (sizeof (char) * MAX_LINE_LENGTH);

  for(i=0; i<MAX_LINE_LENGTH; i++){
    
    if(s[i] == '='){
      i++;
      /*concatenare choiche con s[i] fino a "\0"*/
      while(i<MAX_LINE_LENGTH){
        strncat(choice, &s[i], 1);
        i++;
      }
      i=MAX_LINE_LENGTH;
    }
  }

  return choice;
}

void methodSelection(FILE *f, char* selection){
  
  if (strcmp(selection, "Throughput") == 0) {
    sendResponseMethod(f, 't');
  } else if (strcmp(selection, "IdleRQ") == 0) {
    sendResponseMethod(f, 'i');
  } else if (strcmp(selection, "AdvertisedWindow") == 0) {
    sendResponseMethod(f, 'a');
  } else if (strcmp(selection, "Timeout") == 0) {
    sendResponseMethod(f, 'e');
  } else if(strcmp(selection, "")){
    printf("Homepage\n");
  }else{
    scanData(selection, f);
  }

}

void scanData(char* s, FILE*  f){
  int i=0;
  char* choice = malloc (sizeof (char) * MAX_LINE_LENGTH);
  int counter=0;

  int banda;
  int distanza;
  int dimensione;
  int RTT;
  int EstimatedRTT;
  char* protocollo;

  char* primo = malloc (sizeof (char) * MAX_LINE_LENGTH);
  char* secondo = malloc (sizeof (char) * MAX_LINE_LENGTH);
  char* terzo= malloc (sizeof (char) * MAX_LINE_LENGTH);
  char* quarto= malloc (sizeof (char) * MAX_LINE_LENGTH);

  printf("\n\n%s\n\n",s);

  /*for(i=0; i<MAX_LINE_LENGTH; i++){
    if(i=0){
      while(s[i]!='&'){
        strncat(choice, &s[i], 1);
        i++;
      }
      strncpy(primo, choice, sizeof(choice));
      printf("%s\n", choice);
    }
    
    if(s[i] == '='){
      i++;
      while((s[i]!='&') || (i < MAX_LINE_LENGTH)){
        strncat(choice, &s[i], 1);
        i++;
      }

      printf("%s\n", choice);
      if(counter == 0){
        strncpy(secondo, choice, sizeof(choice));
      }else if(counter == 1){
        strncpy(terzo, choice, sizeof(choice));
      }else if(counter == 2){
        strncpy(quarto, choice, sizeof(choice));
      }
      counter++;
    }
  }*/

  if(strcmp(terzo, "Invia+AdvertisedWindow")){
    banda = (int)primo;
    RTT = (int)secondo;

    printf("A: %d\n%d\n", banda, RTT);

    //advertisedWindow(f, banda, RTT);
  }else if(strcmp(terzo, "Invia+Timeout")){
    RTT = (int)primo;
    EstimatedRTT = (int)secondo;

    printf("TT: %d\n%d\n", RTT, EstimatedRTT);

    //timeout(f, RTT, EstimatedRTT);
  }else if(strcmp(terzo, "Invia+Throughput")){
    banda = (int)primo;
    strncpy(protocollo, secondo, sizeof(secondo));

    printf("T: %d\n%s\n", banda, protocollo);

    //throughput(f, banda, protocollo);
  }else if(strcmp(quarto, "Invia+IdleRQ")){
    banda = (int)primo;
    distanza = (int)secondo;
    dimensione = (int)terzo;

    printf("I: %d\n%d\n%d\n", banda, distanza, dimensione);

    //idleRQ(f, banda, distanza, dimensione);
  }
}

void sendResponseMethod(FILE* f, char c){

  const char htmlResponseThroughput[] = "\r\n <html>\r\n <head>\r\n <title>Protocols calculations Throughput</title>\r\n </head>\r\n <body>\r\n"
  "<h1>Inserire i dati per calcolare il throughput: </h1>\r\n <form action='' method='get'> Banda: <input type='number' id='band' name='band' value='100'></br>\r\n"
  "<input type='radio' id='TCP' name='Protocol' value='TCP'> TCP</br>\r\n <input type='radio' id='UDP' name='Protocol' value='UDP'> UDP</br>\r\n"
  "<input type='submit' id='T' name='T' value='Invia Throughput'></form>\r\n"
  "</body>\r\n </html>\r\n\r\n";

  const char htmlResponseIdleRQ[] = "\r\n <html>\r\n <head>\r\n <title>Protocols calculations IdleRQ</title>\r\n </head>\r\n <body>\r\n"
  "<h1>Inserire i dati per calcolare efficenza di utilizzo (U) e Finestra ottimale (window): </h1>\r\n <form action='' method='get'> Banda: <input type='number' id='band' name='band' value='100'></br>\r\n"
  "Distanza: <input type='number' id='d' name='distanza' value='100'></br>\r\n Dimensione del Frame: <input type='number' id='df' name='DimensioneFrame' value='100'></br>\r\n"
  "<input type='submit' id='I' name='I' value='Invia IdleRQ'></form>\r\n"
  "</body>\r\n </html>\r\n\r\n";

  const char htmlResponseAdvertisedWindow[] = "\r\n <html>\r\n <head>\r\n <title>Protocols calculations AdvertisedWindow</title>\r\n </head>\r\n <body>\r\n"
  "<h1>Inserire i dati per calcolare il valore della finestra ottimale: </h1>\r\n <form action='' method='get'> Banda: <input type='number' id='band' name='band' value='100'></br>\r\n"
  "RTT: <input type='number' id='RTT' name='RTT' value='10'></br>\r\n"
  "<input type='submit' id='A' name='A' value='Invia AdvertisedWindow'></form>\r\n"
  "</body>\r\n </html>\r\n\r\n";

  const char htmlResponseTimeout[] = "\r\n <html>\r\n <head>\r\n <title>Protocols calculations Timeout</title>\r\n </head>\r\n <body>\r\n"
  "<h1>Inserire i dati per calcolare il valore del RTT stimato e il Timeout: </h1>\r\n <form action='' method='get'> RTT: <input type='number' id='RTT' name='RTT' value='10'></br>\r\n"
  "Estimated RTT (precedente): <input type='number' id='ERTT' name='ERTT' value='10'></br>\r\n"
  "<input type='submit' id='TT' name='TT' value='Invia Timeout'></form>\r\n"
  "</body>\r\n </html>\r\n\r\n";

  if(c=='t'){
    fprintf(f, htmlResponseThroughput);

    methodSelection(f, scanString(getRequest(f)));

  }else if(c=='i'){
    fprintf(f, htmlResponseIdleRQ);

    methodSelection(f, scanString(getRequest(f)));
  }else if(c=='a'){
    fprintf(f, htmlResponseAdvertisedWindow);

    methodSelection(f, scanString(getRequest(f)));
  }else if(c=='e'){
    fprintf(f, htmlResponseTimeout);

    methodSelection(f, scanString(getRequest(f)));
  }else{
    die("Invalid Method Selected");
  }

}

//funzione usata in caso di erroe per terminare il programma e stampare un messaggio di errore
void die(char *error) {
    
    fprintf(stderr, "%s.\n", error);
    exit(1);

}