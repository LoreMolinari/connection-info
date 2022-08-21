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
void idleRQ(int);

int main() {

    struct sockaddr_in server_ip_port;
    int server_ip_port_length = sizeof(server_ip_port);

    int clientdescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (clientdescriptor < 0) die("socket() error.\n");
    printf("socket() ok.\n");

    server_ip_port.sin_family = AF_INET; 
    server_ip_port.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_ip_port.sin_port = htons(PORT);

    if(connect(clientdescriptor, (struct sockaddr *) &server_ip_port, server_ip_port_length) < 0) die("connect() error.\n");
    printf("connect() ok.\n");

    handleClient(clientdescriptor);

    close(clientdescriptor);

    return 0;
}


//funzione per definire la gestione delle azioni che può compiere il client
void handleClient(int clientdescriptor) {

    char sendbuff[MAXLEN];
    memset(sendbuff, 0, MAXLEN);

    int exit = 0;

    while(!exit) {

        printf("Scegliere il metodo da eseguire: (throughput = T) (idleRQ = I) (exit = E)\n");
        scanf("%s", sendbuff);

        //invio della scelta del servizio da parte del client
        send(clientdescriptor, &sendbuff, MAXLEN, 0);

        if (strcmp(sendbuff, "T") == 0) {
            printf("\n########################################################################\n");
            printf("Calcolo del throughput\n");
            throughput(clientdescriptor); 
            printf("########################################################################\n\n");
        } else if (strcmp(sendbuff, "I") == 0) {
            printf("\n########################################################################\n");
            printf("Calcolo del idleRQ\n");
            idleRQ(clientdescriptor); 
            printf("########################################################################\n\n");
        } else if (strcmp(sendbuff, "E") == 0) {
            exit = 1;
        } else {
            printf("Scegliere una lettere tra quelle indicate nel messaggio precedente.\n");
        }

    }

}


//funzione per gestire il client nel calcolo del throughput
void throughput(int clientdescriptor) {

    char sendbuff[MAXLEN];
    memset(sendbuff, 0, MAXLEN);

    int check = 0;

    float band = 0;
    float throughput = 0;

    printf("Inserire il valore della banda nominale (in Mbps): ");
    scanf("%f", &band);
    //invio al server del valore acquisito per la banda nominale
    send(clientdescriptor, &band, sizeof(float), 0);

    while(!check) {

        printf("Protocollo utilizzato (TCP/UDP): ");
        scanf("%s", &sendbuff);
        //invio al server del valore acquisito per indicare il protocollo usato
        send(clientdescriptor, &sendbuff, MAXLEN, 0);

        if (strcmp(sendbuff, "TCP") == 0 || strcmp(sendbuff, "UDP") == 0) {
            check = 1;

            //recezione del valore del throughput calcolato dal server
            recv(clientdescriptor, &throughput, sizeof(float), 0);
            
            printf("Valore del throughput (in Mbps): ");
            printf("%f\n", throughput);
        } else {
            check = 0;
            printf("Scegliere un protocollo tra quelli indicati nel messaggio precedente.\n");
        }
    }

}


//funzione per gestire il client nel calcolo dell’efficienza di utilizzo e della finestra ottimale in idleRQ
void idleRQ(int clientdescriptor) {

    float d = 0;
    float U = 0; 
    int frame = 0;  
    float band = 0;
    float window = 0;

    printf("Inserire il valore della dimensione della PDU (in bit): ");
    scanf("%d", &frame);
    //invio al server del valore acquisito per la PDU
    send(clientdescriptor, &frame, sizeof(int), 0);

    printf("Inserire il valore della distanza (in metri): ");
    scanf("%f", &d);
    //invio al server del valore acquisito per distanza
    send(clientdescriptor, &d, sizeof(float), 0);

    printf("Inserire il valore della banda nominale (in bps): ");
    scanf("%f", &band);
    //invio al server del valore acquisito per la banda 
    send(clientdescriptor, &band, sizeof(float), 0);

    //recezione del valore dell'efficienza di utilizzo calcolata 
    recv(clientdescriptor, &U, sizeof(float), 0);
    printf("Valore dell'efficienza di utilizzo: ");
    printf("%f\n", U);

    //recezione del valore della finestra ottimale calcolata
    recv(clientdescriptor, &window, sizeof(float), 0);
    printf("Valore della finestra ottimale: ");
    printf("%f\n", window);

}


//funzione usata in caso di erroe per terminare il programma e stampare un messaggio di errore
void die(char *error) {
    
    fprintf(stderr, "%s.\n", error);
    exit(1);

}