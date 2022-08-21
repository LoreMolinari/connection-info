#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define LISTEN 5
#define MAXLEN 255

void die(char *);
void handleClient(int);
void throughput(int);

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
        printf("------- new client connected. -------\n");

        pid_t pid = fork();

        if (pid < 0) {
            die("fork() error.\n"); 
        } else if (pid > 0) {
            close(clientdescriptor); 
        } else if (pid == 0) {
            close(serverdescriptor);
            handleClient(clientdescriptor);
            printf("------- client disconnected. -------\n");
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

    int check = 0;

    while(!check) {

        //acquisizione della scelta del servizio del client
        recv(clientdescriptor, &recvbuff, MAXLEN, 0); 

        if (strcmp(recvbuff, "T") == 0) {
            check = 1;
            throughput(clientdescriptor); 
        } else if (strcmp(recvbuff, "E") == 0) {
            check = 1;
        } else {
            check = 0;
        }

    }

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

}


//funzione usata in caso di erroe per terminare il programma e stampare un messaggio di errore
void die(char *error) {
    
    fprintf(stderr, "%s.\n", error);
    exit(1);

}