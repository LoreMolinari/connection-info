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

#define PORT 9090
#define LISTEN 5
#define MAXLEN 2048
#define MAX_LINE_LENGTH (1024)

char* scanString(char* s);
void die(char *error);
float throughput(int banda, char* protocollo);
int advertisedWindow(int banda, int RTT);
const char* timeout(int RTT, float EstimatedRTT);
const char* idleRQ(int banda, int distanza, int dimensione);

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

    sendResponse(f, getRequest(f), "");

    fclose(f);

    // stdout needs to be flushed in order for heroku to read the logs
    fflush(stdout);
    printf("fflush()\n");

}

void sendResponse(FILE *f, char* key, char *data){ 

  char html[MAXLEN] = "\r\n <html>\r\n<head>\r\n <title>Protocols calculations</title>\r\n <link rel='icon' href='data:,'> \r\n <head>\r\n <body>\r\n<p>";
  char htmlBottom[MAXLEN] = "</p>\r\n</body>\r\n </html>\r\n";

  if(strcmp(data, "") == 0){
    data = "<h1>Scegliere il metodo da eseguire: </h1>\r\n <form action='' method='get'> <input type='submit' id='t' name='choice' value='Throughput'>\r\n"
    "<input type='submit' id='i' name='choice' value='IdleRQ'>\r\n <input type='submit' id='w' name='choice' value='AdvertisedWindow'>\r\n"
    "<input type='submit' id='TT' name='choice' value='Timeout'></form>\r\n";

    fprintf(f, "HTTP/1.1 200 OK\r\n");
    fprintf(f, "Content-Type: text/html\r\n");
  }

  strncat(html, data, MAXLEN);
  strncat(html, &htmlBottom, sizeof(htmlBottom));
  fprintf(f, html);
  
  scanString(key);

  if(strcmp(key, "") == 0){
    methodSelection(f, "");
  }else{
    methodSelection(f, scanString(key));
  }

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

    char data[MAXLEN] = "<h1>Inserire i dati per calcolare il throughput: </h1>\r\n <form action='' method='get'> Banda: <input type='number' id='band' name='band' value='100'></br>\r\n"
    "<input type='radio' id='TCP' name='Protocol' value='TCP'> TCP</br>\r\n <input type='radio' id='UDP' name='Protocol' value='UDP'> UDP</br>\r\n"
    "<input type='submit' id='T' name='T' value='Throughput'></form>\r\n";

    sendResponse(f, "", data);

  } else if (strcmp(selection, "IdleRQ") == 0) {

    char data[MAXLEN] = "<h1>Inserire i dati per calcolare efficenza di utilizzo (U) e Finestra ottimale (window): </h1>\r\n <form action='' method='get'> Banda: <input type='number' id='band' name='band' value='100'></br>\r\n"
    "Distanza: <input type='number' id='d' name='distanza' value='100'></br>\r\n Dimensione del Frame: <input type='number' id='df' name='DimensioneFrame' value='100'></br>\r\n"
    "<input type='submit' id='I' name='I' value='IdleRQ'></form>\r\n";

    sendResponse(f, "", data);

  } else if (strcmp(selection, "AdvertisedWindow") == 0) {

    char data[MAXLEN] = "<h1>Inserire i dati per calcolare il valore della finestra ottimale: </h1>\r\n <form action='' method='get'> Banda: <input type='number' id='band' name='band' value='100'></br>\r\n"
    "RTT: <input type='number' id='RTT' name='RTT' value='10'></br>\r\n"
    "<input type='submit' id='A' name='A' value='AdvertisedWindow'></form>\r\n";

    sendResponse(f, "", data);

  } else if (strcmp(selection, "Timeout") == 0) {

    char data[MAXLEN] = "<h1>Inserire i dati per calcolare il valore del RTT stimato e il Timeout: </h1>\r\n <form action='' method='get'> RTT: <input type='number' id='RTT' name='RTT' value='10'></br>\r\n"
    "Estimated RTT (precedente): <input type='number' id='ERTT' name='ERTT' value='10'></br>\r\n"
    "<input type='submit' id='TT' name='TT' value='Timeout'></form>\r\n";

    sendResponse(f, "", data);

  } else{
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

  if(strcmp(s,"")!=0){
    for(i=0; i<MAX_LINE_LENGTH; i++){
      if(i==0){
        while(s[i]!='&'){
          strncat(choice, &s[i], 1);
          i++;
        }
        strncpy(primo, choice, sizeof(choice));

        free(choice);
        choice =  malloc (sizeof (char) * MAX_LINE_LENGTH);
      }
      
      if(s[i] == '='){
        i++;
        while(isalnum(s[i])){
          strncat(choice, &s[i], 1);
          i++;
        }

        if(counter == 0){

          strncpy(secondo, choice, sizeof(choice));
          free(choice);
          choice =  malloc (sizeof (char) * MAX_LINE_LENGTH);

        }else if(counter == 1){

          strncpy(terzo, choice, 1024);
          free(choice);
          choice =  malloc (sizeof (char) * MAX_LINE_LENGTH);

        }else if(counter == 2){

          strncpy(quarto, choice, 1024);

        }
        counter++;
      }
    }


    free(choice);

    if(strcmp(terzo, "AdvertisedWindow")==0){
      sscanf(primo, "%d", &banda);
      sscanf(secondo, "%d", &RTT);

      free(primo);
      free(secondo);
      free(terzo);
      free(quarto);

      //per stampare il risultato ottenuto usando il protocollo HTTP 1.1
      char data[MAXLEN] = "Valore del Advertised Window: ";
      char toString[MAXLEN];
      sprintf(toString, "%d", advertisedWindow(banda, RTT));
      strncat(data, &toString, MAXLEN);

      sendResponse(f, "", data);

    }else if(strcmp(terzo, "Timeout")==0){
      sscanf(primo, "%d", &RTT);
      sscanf(secondo, "%d", &EstimatedRTT);

      free(primo);
      free(secondo);
      free(terzo);
      free(quarto);

      //per stampare il risultato ottenuto usando il protocollo HTTP 1.1
      sendResponse(f, "", timeout(RTT, (float)EstimatedRTT));

      //timeout(f, RTT, (float)EstimatedRTT);
    }else if(strcmp(terzo, "Throughput")==0){
      sscanf(primo, "%d", &banda);

      free(primo);
      free(terzo);
      free(quarto);

      //per stampare il risultato ottenuto usando il protocollo HTTP 1.1
      char data[MAXLEN] = "Valore del throughput: ";
      char toString[MAXLEN];
      sprintf(toString, "%f", throughput(banda, secondo));
      strncat(data, &toString, MAXLEN);
      free(secondo);
      sendResponse(f, "", data);

    }else if(strcmp(quarto, "IdleRQ")==0){
      sscanf(primo, "%d", &banda);
      sscanf(secondo, "%d", &distanza);
      sscanf(terzo, "%d", &dimensione);
      
      free(primo);
      free(secondo);
      free(terzo);
      free(quarto);

      //per stampare il risultato ottenuto usando il protocollo HTTP 1.1
      sendResponse(f, "", idleRQ(banda, distanza, dimensione));
    }else{
      die("Protcol Error");
    }
  }
  
}


int advertisedWindow(int banda, int RTT){

  return banda * RTT;

}

const char* timeout(int RTT, float EstimatedRTT){

  float timeout = 0;
  float x = 0.1;

  EstimatedRTT = ((1-x) * EstimatedRTT) + (x * RTT);
  timeout = EstimatedRTT * 2;

  char data[MAXLEN] = "Valore del Timeout: ";
  char dataTemp[MAXLEN] = ", Valore del RTT Stimato: ";
  char toString[MAXLEN];
  sprintf(toString, "%f", timeout);
  strncat(data, &toString, MAXLEN);
  strncat(data, &dataTemp, MAXLEN);
  sprintf(toString, "%f", EstimatedRTT);
  strncat(data, &toString, MAXLEN);

  char* data2 = data;
  return data2;
}

const char* idleRQ(int banda, int distanza, int dimensione){

  float Tix = 0; //tempo di trasmissione del frame
  float Tp = 0; //ritardo di propagazione
  float Tt = 0; //tempo totale che intercorre tra l’invio di un frame e il successivo
  float U = 0; //efficienza di utilizzo
  float window =  0;
  
  Tix = (float)dimensione/(float)banda;
  Tp = (float)distanza/(float)banda;

  Tt = Tix + 2*Tp;
  U = Tix/(Tix + 2*Tp);

  window = Tt * (float)banda;

  char data[MAXLEN] = "Valore della finestra: ";
  char dataTemp[MAXLEN] = ", Valore dell'efficenza del canale: ";
  char toString[MAXLEN];
  sprintf(toString, "%f", window);
  strncat(data, &toString, MAXLEN);
  strncat(data, &dataTemp, MAXLEN);
  sprintf(toString, "%f", U);
  strncat(data, &toString, MAXLEN);

  char* data2 = data;
  return data2;
  
}

float throughput(int banda, char* protocollo){

  char* data;

  float throughput = 0;
  float overheadEthernet = 18;
  float overheadIp = 20;
  float frameEthernet = 1500;
  float overheadTCP = 20;
  float overheadUDP = 8;

  if (strcmp(protocollo, "TCP") == 0) {
    //(1500 - 40)/(1500 + 18)
    throughput = (frameEthernet - (overheadTCP + overheadIp)) / (frameEthernet + overheadEthernet);
    throughput = throughput * (float)banda;
  } else if (strcmp(protocollo, "UDP") == 0) {
    //(1500 - 28)/(1500 + 18)
    throughput = (frameEthernet - (overheadUDP + overheadIp)) / (frameEthernet + overheadEthernet);
    throughput = throughput * (float)banda;
  }

  return throughput;
}

//funzione usata in caso di erroe per terminare il programma e stampare un messaggio di errore
void die(char *error) {
    
    fprintf(stderr, "%s.\n", error);
    exit(1);

}