#include <stdio.h>;
#include <stdlib.h>;
#include <string.h>;
#include <sys/socket.h>;
#include <netinet/in.h>;

#define PORT 8080

void die(char *);

int main() {

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

        char choice[40];
        printf("Scegliere il metodo da eseguire: (parametri, info, simula)\n");
        scanf("%s", choice);
        printf("Scelta selezionata: %s.\n", choice);
        send(socketdescriptor, &choice, sizeof(choice), 0);

    }

    close(socketdescriptor);
    return 0;

}


void die(char *error) {
    fprintf(stderr, "%s.\n", error);
    exit(1);
}