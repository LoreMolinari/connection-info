#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8989
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

        printf("Scegliere il metodo da eseguire: (throughput = T)\n");
        scanf("%s", sendbuff);
        send(socketdescriptor, &sendbuff, MAXLEN, 0);

        if (strcmp(sendbuff, "T") == 0) {
            printf("Gestione del servizio: calcolo del throughput\n");
            throughput(socketdescriptor); 
        } else {
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
    int throughput = 0;

    printf("Inserire il valore della banda nominale (in Mbps): ");
    scanf("%d", &bandaNominale);
    send(socketdescriptor, &bandaNominale, sizeof(int), 0);

    printf("Protocollo utilizzato (TCP/UDP): ");
    scanf("%s", &sendbuff);
    send(socketdescriptor, &sendbuff, MAXLEN, 0);

    if (strcmp(sendbuff, "TCP") == 0 || strcmp(sendbuff, "UDP") == 0) {
        printf("Valore del throughput (in Mbps): ");
        recv(socketdescriptor, &throughput, sizeof(int), 0);
        printf("%d\n", throughput);
    } else {
        printf("Scelta non possibile\n");
    }

}


void die(char *error) {
    fprintf(stderr, "%s.\n", error);
    exit(1);
}