#include <socket.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char * argv){
    //DEFINIZIONE VARIABILI
    struct sockaddr_in local, client;

    local.sin_family = AF_INET;
    local.sin_port = htons(ServPort);
    local.sin_address.s_addr = inet_addr(ServIP)

    //inizializzazione Socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if ((clientSocket == -1){
        printf("Errore nella creazione del socket");
        exit(-1);
    }

    //connessione al server
    int serverSocket = connect(clientSocket, (struct sockaddr * ) & ServAddr, sizeof(ServAddr));
    if (serverSocket == -1){
        printf("Errore nella connessione al server");
        exit(-1);
    }

    char scelta[40];
    printf("Scegliere il metodo da eseguire: (parametriConnessione, infoConnessione, simulaConnessione)\n");
    scanf("%s", scelta);
    printf("Scelta selezionata: %s.\n", scelta);
    send(serverSocket, &scelta, sizeof(scelta), 0);
    
    if (strcmp(scelta, "parametriConnessione") == 0) {
        extract_param(serverSocket);
    } else if (strcmp(scelta, "infoConnessione") == 0) {
        extract_info(serverSocket);
    } else if(strcmp(scelta, "simulaConnessione") == 0){
        simulation(serverSocket);
    }else {
        printf("Errore nella richiesta, funzione non implementata, scegli tra quelle disponibili");
        exit(-1);
    }
    
    close(clientSocket);
}

void extract_param(int serverSocket){
    recv(serverSocket, &msg, sizeof(msg), 0);
    printf("%s", msg);

    //Andranno poi gestite le richieste in base al server
}

void extract_info(int serverSocket){
    recv(serverSocket, &msg, sizeof(msg), 0);
    printf("%s", msg);

    //Andranno poi gestite le richieste in base al server
}

void simulation(int serverSocket){
    recv(serverSocket, &msg, sizeof(msg), 0);
    printf("%s", msg);

    char param[40];
    recv(serverSocket, &msg, sizeof(msg), 0);
    printf("%s", msg);
    scanf("%s", param);
    send(serverSocket, &param, sizeof(param), 0);

    recv(serverSocket, &msg, sizeof(msg), 0);
    printf("%s", msg);
    scanf("%s", param);
    send(serverSocket, &param, sizeof(param), 0);

    recv(serverSocket, &msg, sizeof(msg), 0);
    printf("%s", msg);
    scanf("%s", param);
    send(serverSocket, &param, sizeof(param), 0);

    //Andranno poi gestite le richieste in base al server
}