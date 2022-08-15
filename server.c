#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9090
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
            die("fork() error"); 
        } else if (pid > 0) {
            //Processo padre: chiudo il socket slave
            close(clientdescriptor); 
        } else {
            //Processo figlio: chiudo il socket master
            close(serverdescriptor);
            memset(choice, 0, 10);

            printf("Client %d connesso\n",clientdescriptor); //stampo id del socket

            while(1){
                //Gestione del client acquisito
                printf("In attesa della scelta del client\n");
                recv(clientdescriptor, &recvbuff, MAXLEN, 0);

                if (strcmp(recvbuff, "T") == 0) {
                    //printf("Gestione del servizio: calcolo del throughput\n");
                    throughput(clientdescriptor); 
                }else if(strcmp(recvbuff, "U") == 0){
                    //printf("Gestione del servizio: calcolo dell'efficenza\n");
                    channelEfficency(clientdescriptor); 
                }
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
    memset(recvbuff, 0, MAXLEN);

    int bandaNominale = 0;
    float throughput = 0;
    int overheadEthernet = 18;
    int overheadIp = 20;
    int frameEthernet = 1500;
    int overheadTCP = 20;
    int overheadUDP = 8;
    

    recv(clientdescriptor, &bandaNominale, sizeof(int), 0);
    //printf("Banda nominale: %d\n", bandaNominale); //pv
    recv(clientdescriptor, &recvbuff, MAXLEN, 0);
    //printf("Protocollo: %s\n", recvbuff);   //pv

    if (strcmp(recvbuff, "TCP") == 0) {
        throughput = (float)(frameEthernet - (overheadTCP + overheadIp)) / (float)(frameEthernet + overheadEthernet);
        throughput = throughput * (float)bandaNominale;
    } else if (strcmp(recvbuff, "UDP") == 0) {
        throughput = (float)(frameEthernet - (overheadUDP + overheadIp)) / (float)(frameEthernet + overheadEthernet);
        throughput = throughput * (float)bandaNominale;
    }

    //printf("%f\n", throughput); //provvisorio
    send(clientdescriptor, &throughput, sizeof(float), 0);

}

void channelEfficency (int clientdescriptor){

    int TIX = 0;
    int Tt = 0;
    int RTT = 0;
    int U = 0;
    float TassoU = 0;

    recv(clientdescriptor, &TIX, sizeof(int), 0);
    //printf("Tempo di invio del pacchetto: %d\n", TIX); //pv
    recv(clientdescriptor, &Tt, sizeof(int), 0);
    //printf("Tempo tra invio di un frame e il successivo: %d\n", Tt);   //pv
    recv(clientdescriptor, &RTT, sizeof(int), 0);
    //printf("Tempo di propagazione: %d\n", Tt);   //pv


    U = TIX/Tt;
    TassoU = ((float)TIX/(float)(TIX+2*RTT))*100;

    //printf("%d\n%f\n", U, TassoU); //provvisorio
    send(clientdescriptor, &U, sizeof(int), 0);
    send(clientdescriptor, &TassoU, sizeof(float), 0);
}


void die(char *error) {
    fprintf(stderr, "%s.\n", error);
    exit(1);
}