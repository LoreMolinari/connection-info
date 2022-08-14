#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8989
#define LISTEN 5
#define MAXLEN 255

void die(char *);
void throughput(int);

int main() {

    char recvbuff[MAXLEN];
    memset(recvbuff, 0, MAXLEN);

    struct sockaddr_in local;
    int serverdescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (serverdescriptor < 0) die("socket() error");
    printf("socket() ok.\n");

    local.sin_family = AF_INET; 
    local.sin_addr.s_addr = inet_addr("127.0.0.1");
    local.sin_port = htons(PORT);
    
    if(bind(serverdescriptor, (struct sockaddr *) &local, sizeof(local)) < 0) die("bind() error");
    printf("bind() ok.\n");

    if(listen(serverdescriptor, LISTEN) < 0) die("listen() error");
    printf("listen ok.\n");

    char choice[10];

    while(1) {

        int clientdescriptor = accept(serverdescriptor, NULL, NULL);
        if (clientdescriptor < 0) die("accept() error");
        printf("accept() ok.\n");

        int pid = fork();

        if (pid < 0) {
            die("frok() error"); 
        } else if (pid > 0) {
            //Processo padre: chiudo il socket slave
            close(clientdescriptor); 
        } else {
            //Processo figlio: chiudo il socket master
            close(serverdescriptor);
            memset(choice, 0, 10);

            //Gestione del client acquisito
            printf("In attesa della scelta del client\n");
            recv(clientdescriptor, &recvbuff, MAXLEN, 0);

            if (strcmp(recvbuff, "T") == 0) {
                printf("Gestione del servizio: calcolo del throughput\n");
                throughput(clientdescriptor); 
            }

            printf("Servizio terminato\n");
            
            exit(0);
        }

    }

    close(serverdescriptor);
    return 0;

}


void throughput(int clientdescriptor) {
    
    char recvbuff[MAXLEN];
    //memset(recvbuff, 0, MAXLEN);

    int bandaNominale = 0;
    int throughput = 0;
    int overheadEthernet = 18;
    int overheadIp = 20;
    int frameEthernet = 1500;
    int overheadTCP = 20;
    int overheadUDP = 8;
    

    recv(clientdescriptor, &bandaNominale, sizeof(int), 0);
    printf("Banda nominale: %d\n", bandaNominale); //pv
    recv(clientdescriptor, &recvbuff, MAXLEN, 0);
    printf("Protocollo: %s\n", recvbuff);   //pv

    if (strcmp(recvbuff, "TCP") == 0) {
        throughput = (frameEthernet - (overheadTCP + overheadIp)) / (frameEthernet + overheadEthernet);
        throughput = throughput/100 * bandaNominale;
    } else if (strcmp(recvbuff, "UDP") == 0) {
        throughput = (frameEthernet - (overheadUDP + overheadIp)) / (frameEthernet + overheadEthernet);
        throughput = throughput/100 * bandaNominale;
    }

    printf("%d\n", throughput); //provvisorio
    send(clientdescriptor, &throughput, sizeof(throughput), 0);

}


void die(char *error) {
    fprintf(stderr, "%s.\n", error);
    exit(1);
}