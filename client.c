#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9090
#define MAXLEN 255

void die(char *);

int main() {

    char sendbuff[MAXLEN];
    char recvbuff[MAXLEN];
    memset(sendbuff, 0, MAXLEN);
    memset(recvbuff, 0, MAXLEN);

    struct sockaddr_in local;
    int socketdescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketdescriptor < 0) die("socket() error");
    printf("socket() ok.\n");

    local.sin_family = AF_INET; 
    local.sin_addr.s_addr = inet_addr("127.0.0.1");
    local.sin_port = htons(PORT);

    if(connect(socketdescriptor, (struct sockaddr *) &local, sizeof(local)) < 0) die("connect() error");
    printf("connect() ok.\n");

    while(1) {

        printf("Scegliere il metodo da eseguire: (throughput = T) (Efficenza del canale e tasso utilizzo = U) (Advertised Window = A) (Timeout = R)\n");
        scanf("%s", sendbuff);
        send(socketdescriptor, &sendbuff, MAXLEN, 0);

        if (strcmp(sendbuff, "T") == 0) {
            printf("Gestione del servizio: calcolo del throughput\n");
            throughput(socketdescriptor); 
        } else if(strcmp(sendbuff, "U") == 0){
            printf("Gestione del servizio: calcolo dell'efficenza\n");
            channelEfficency(socketdescriptor); 
        } else if(strcmp(sendbuff, "A") == 0){
            printf("Gestione del servizio: calcolo della finestra ottimale\n");
            avertisedWindow(socketdescriptor); 
        }else if(strcmp(sendbuff, "R") == 0){
            printf("Gestione del servizio: calcolo del timeout\n");
            RTT(socketdescriptor); 
        }else {
            printf("Questa funzione non è stata implementata\n");
        }

    }

    close(socketdescriptor);
    return 0;

}


void throughput(int socketdescriptor) {
    
    char sendbuff[MAXLEN];
    char recvbuff[MAXLEN];
    memset(sendbuff, 0, MAXLEN);
    memset(recvbuff, 0, MAXLEN);

    int bandaNominale = 0;
    float throughput = 0;

    printf("Inserire il valore della banda nominale (in Mbps): ");
    scanf("%d", &bandaNominale);
    send(socketdescriptor, &bandaNominale, sizeof(int), 0);

    printf("Protocollo utilizzato (TCP/UDP): ");
    scanf("%s", &sendbuff);
    send(socketdescriptor, &sendbuff, MAXLEN, 0);

    if (strcmp(sendbuff, "TCP") == 0 || strcmp(sendbuff, "UDP") == 0) {
        printf("Valore del throughput (in Mbps): ");
        recv(socketdescriptor, &throughput, sizeof(float), 0);
        printf("%f\n\n", throughput);
    } else {
        printf("Scelta non possibile\n\n");
    }

}

void channelEfficency(int socketdescriptor){

    int TIX = 0;
    int Tt = 0;
    int RTT = 0;
    int U = 0;
    float TassoU = 0;

    printf("Inserire il tempo di invio del pacchetto: ");
    scanf("%d", &TIX);
    send(socketdescriptor, &TIX, sizeof(int), 0);

    printf("Inserire il tempo che intercorre tra un invio di un frame e il successivo: ");
    scanf("%d", &Tt);
    send(socketdescriptor, &Tt, sizeof(int), 0);

    printf("Inserire il tempo di propagazione del canale (RTT): ");
    scanf("%d", &RTT);
    send(socketdescriptor, &RTT, sizeof(int), 0);

    printf("Valore dell'efficenza di utilizzo del canale: ");
    recv(socketdescriptor, &U, sizeof(int), 0);
    printf("%d\n", U);

    printf("Valore del tasso di utilizzo del canale: ");
    recv(socketdescriptor, &TassoU, sizeof(float), 0);
    printf("%5.2f %\n\n", TassoU);
}

void avertisedWindow(int socketdescriptor){

    int RTT = 0;
    int width = 0;
    int AW = 0;

    printf("Inserire il tempo di propagazione del canale(RTT): ");
    scanf("%d", &RTT);
    send(socketdescriptor, &RTT, sizeof(int),0);
    
    printf("Inserire la larghezza di banda del canale: ");
    scanf("%d", &width);
    send(socketdescriptor, &width, sizeof(int), 0);

    printf("Il valore della finestra ottimale è: ");
    recv(socketdescriptor, &AW, sizeof(int), 0);
    printf("%d\n\n", AW);
}

void RTT(int socketdescriptor){
    
    int currentTime = 0;
    int sendTime = 0;

    float EstimatedRTT = 1;
    float Timeout = 0;
    float Error = 0;

    printf("Inserire il tempo corrente: ");
    scanf("%d", &currentTime);
    send(socketdescriptor, &currentTime, sizeof(int),0);

    printf("Inserire il tempo di invio del pacchetto: ");
    scanf("%d", &sendTime);
    send(socketdescriptor, &sendTime, sizeof(int),0);

    
    recv(socketdescriptor, &EstimatedRTT, sizeof(float), 0);
    recv(socketdescriptor, &Error, sizeof(float), 0);
    recv(socketdescriptor, &Timeout, sizeof(float), 0);
    printf("Estimated RTT: %5.2f,\t Errore: %5.2f,\t Timeout: %5.2f\n\n", EstimatedRTT, Error, Timeout);

}


void die(char *error) {
    fprintf(stderr, "%s.\n", error);
    exit(1);
}