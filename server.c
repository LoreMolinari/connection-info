#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "getRequest.h"

#define PORT 8080
#define WEBPORT 9090
#define LISTEN 5
#define MAXLEN 255

void die(char *);
void handleClient(int);
void throughput(int);
void window(int);
void timeout(int);
void printResult(char *);

int main() {

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

    while(1) {
        
        int clientdescriptor = accept(serverdescriptor, NULL, NULL);
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


//funzione per definire la gestione del client acquisito
void handleClient(int clientdescriptor) {

    char recvbuff[MAXLEN];
    memset(recvbuff, 0, MAXLEN);

    int exit = 0;

    while(!exit) {

        //acquisizione della scelta del servizio del client
        recv(clientdescriptor, &recvbuff, MAXLEN, 0); 

        if (strcmp(recvbuff, "T") == 0) {
            exit = 0;
            throughput(clientdescriptor);
        } else if (strcmp(recvbuff, "I") == 0) {
            exit = 0;
            idleRQ(clientdescriptor);
        } else if (strcmp(recvbuff, "W") == 0) {
            exit = 0;
            window(clientdescriptor);
        } else if (strcmp(recvbuff, "TT") == 0) {
            exit = 0;
            timeout(clientdescriptor);
        } else if (strcmp(recvbuff, "E") == 0) {
            exit = 1;
        } else {
            exit = 0;
        }

    }

}


void printResult(char *data) {

    struct sockaddr_in bind_ip_port;
    int bind_ip_port_length = sizeof(bind_ip_port);

    int serverdescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (serverdescriptor < 0) die("web socket() error.\n");
    printf("web socket() ok.\n");

    bind_ip_port.sin_family = AF_INET; 
    bind_ip_port.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind_ip_port.sin_port = htons(WEBPORT);

    int enable = 1;
    setsockopt(serverdescriptor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    if(bind(serverdescriptor, (struct sockaddr *) &bind_ip_port, bind_ip_port_length) < 0) {
        printf("Oh dear, something went wrong with bind()! %s\n", strerror(errno));
        die("web bind() error.\n");
    }
    printf("web bind() ok.\n");

    if(listen(serverdescriptor, LISTEN) < 0) die("web listen() error.\n");
    printf("web listen ok.\n");

    int clientdescriptor = accept(serverdescriptor, NULL, NULL);
    if (clientdescriptor < 0) die("web accept() error.\n");
    printf("web accept() ok.\n");
    
    printf("---------- new client connected ----------\n");
    
    FILE* f = fdopen(clientdescriptor, "w+");
    if (f == NULL) die("fdopen() error.\n");
    printf("fdopen() ok.\n");

    getRequest(f);
    fflush(stdout);

    char html[MAXLEN] = "\r\n <html>\r\n <body>\r\n<p>";
    char htmlBottom[MAXLEN] = "</p>\r\n</body>\r\n </html>\r\n";

    strncat(html, data, MAXLEN);
    strncat(html, &htmlBottom, sizeof(htmlBottom));

    printf("\n%s\n", html);

    fprintf(f, "HTTP/1.1 200 OK\r\n");
    fprintf(f, "Content-Type: text/html\r\n");
    fprintf(f, html);

    fclose(f);
    fflush(stdout);

    close(clientdescriptor);
    close(serverdescriptor);

    printf("---------- client disconnected ----------\n");

}


//funzione per implementare il servizio di calcolo del throughput
void throughput(int clientdescriptor) {
    
    char recvbuff[MAXLEN];
    memset(recvbuff, 0, MAXLEN);

    int check = 0;

    float band = 0;
    float throughput = 0;
    float overheadEthernet = 18;
    float overheadIp = 20;
    float frameEthernet = 1500;
    float overheadTCP = 20;
    float overheadUDP = 8;

    //acquisizione del valore della banda nominale
    recv(clientdescriptor, &band, sizeof(float), 0);

    while(!check) {

        //acquisizione del tipo di protocollo usato (TCP oppure UDP)
        recv(clientdescriptor, &recvbuff, MAXLEN, 0);

        if (strcmp(recvbuff, "TCP") == 0) {
            check = 1;
            //(1500 - 40)/(1500 + 18)
            throughput = (frameEthernet - (overheadTCP + overheadIp)) / (frameEthernet + overheadEthernet);
            throughput = throughput * band;
        } else if (strcmp(recvbuff, "UDP") == 0) {
            check = 1;
            //(1500 - 28)/(1500 + 18)
            throughput = (frameEthernet - (overheadUDP + overheadIp)) / (frameEthernet + overheadEthernet);
            throughput = throughput * band;
        } else {
            check = 0;
        }

    }

    //invio del valore del throughput calcolato in base ai parametri ricevuti
    send(clientdescriptor, &throughput, sizeof(float), 0);

    //per stampare il risultato ottenuto usando il protocollo HTTP 1.1
    char data[MAXLEN] = "Valore del throughput: ";
    char toString[MAXLEN];
    gcvt(throughput, 10, toString);
    strncat(data, &toString, MAXLEN);
    printResult(&data);

}


//funzione per il calcolo dell’efficienza di utilizzo e della finestra ottimale in idleRQ
void idleRQ(int clientdescriptor) {

    float Tix = 0; //tempo di trasmissione del frame
    float Tp = 0; //ritardo di propagazione
    float Tt = 0; //tempo totale che intercorre tra l’invio di un frame e il successivo
    float d = 0;
    float U = 0; //efficienza di utilizzo
    int frame = 0; //dimensione del frame 

    float band = 0;
    float window =  0;

    //acquisizione della dimensione del frame ethernet
    recv(clientdescriptor, &frame, sizeof(int), 0);

    //acquisizione del valore della distanza
    recv(clientdescriptor, &d, sizeof(float), 0);

    //acquisizione del valore della banda
    recv(clientdescriptor, &band, sizeof(float), 0);

    Tix = (float)frame/band;
    Tp = d/band;

    Tt = Tix + 2*Tp;
    U = Tix/(Tix + 2*Tp);

    window = Tt * band;

    //invio del valore dell'efficienza calcolato in base ai parametri ricevuti
    send(clientdescriptor, &U, sizeof(float), 0);

    //invio del valore della finestra ottimale calcolato in base ai parametri ricevuti
    send(clientdescriptor, &window, sizeof(float), 0);

    //per stampare il risultato ottenuto usando il protocollo HTTP 1.1
    char data[MAXLEN] = "Valore dell'efficienza: ";
    char toString[MAXLEN];
    gcvt(U, 10, toString);
    strncat(data, &toString, MAXLEN);
    strncat(data, " - Valore della finestra ottimale: ", MAXLEN);
    gcvt(window, 10, toString);
    strncat(data, &toString, MAXLEN);
    printResult(&data);

}


//funzione usata per calcola la finestra ottimale
void window(int clientdescriptor){

    float Tt = 0;
    float band = 0;
    float window = 0;

    //acquisizione del valore tempo totale che intercorre tra l’invio di un frame e il successivo (o RTT)
    recv(clientdescriptor, &Tt, sizeof(float), 0);

    //acquisizione del valore della banda
    recv(clientdescriptor, &band, sizeof(float), 0);

    window = Tt * band;

    //invio del valore della finestra ottimale calcolato in base ai parametri ricevuti
    send(clientdescriptor, &window, sizeof(float), 0);

    //per stampare il risultato ottenuto usando il protocollo HTTP 1.1
    char data[MAXLEN] = "Valore della finestra ottimale: ";
    char toString[MAXLEN];
    gcvt(window, 10, toString);
    strncat(data, &toString, MAXLEN);
    printResult(&data);

}


//funzione usata per il calcolo del timeout
void timeout(int clientdescriptor) {

    float sampleRTT = 0;
    float estimatedRTT = 0;
    float timeout = 0;

    float x = 0.1;

    //acquisizione del valore del sample RTT
    recv(clientdescriptor, &sampleRTT, sizeof(float),0);

    //acquisizione del valore del'estimated RTT precedente
    recv(clientdescriptor, &estimatedRTT, sizeof(float),0);

    estimatedRTT = ((1-x) * estimatedRTT) + (x * sampleRTT);
    timeout = estimatedRTT * 2;

    //invio del valore dell'estimated RTT
    send(clientdescriptor, &estimatedRTT, sizeof(float), 0);

    //invio del valore del timeout calcolato con i parametri dati
    send(clientdescriptor, &timeout, sizeof(float), 0);

    //per stampare il risultato ottenuto usando il protocollo HTTP 1.1
    char data[MAXLEN] = "Valore dell'estimated RTT: ";
    char toString[MAXLEN];
    gcvt(estimatedRTT, 10, toString);
    strncat(data, &toString, MAXLEN);
    strncat(data, " - Valore del timeout: ", MAXLEN);
    gcvt(timeout, 10, toString);
    strncat(data, &toString, MAXLEN);
    printResult(&data);

}


//funzione usata in caso di erroe per terminare il programma e stampare un messaggio di errore
void die(char *error) {
    
    fprintf(stderr, "%s.\n", error);
    exit(1);

}