#include <stdio.h>;
#include <stdlib.h>;
#include <string.h>;
#include <sys/socket.h>;
#include <netinet/in.h>;

#define PORT 8080
#define LISTEN 5

void die(char *);
void extract_param(int);
void extract_info(int);
void simulation(int);

int main() {

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
            recv(clientdescriptor, &choice, sizeof(choice), 0);
            printf("Scelta selezionata: %s\n", choice);

            if (strcmp(choice, "parametri") == 0) {
                printf("Gestione del servizio paramentri\n");
                extract_param(clientdescriptor);
            } else if (strcmp(choice, "info") == 0) {
                printf("Gestione del servizio info\n");
                extract_info(clientdescriptor);
            } else if (strcmp(choice, "simula") == 0) {
                printf("Gestione del servizio simula\n");
                simulation(clientdescriptor); 
            } else {
                char *msg = "Questa funzione non è stata implementata";
                send(clientdescriptor, &msg, sizeof(msg), 0);
            }
            
            exit(0);
        }

    }

    close(serverdescriptor);
    return 0;

}


void extract_param(int clientdescriptor) {
    char *msg = "Verranno estratti i parametri della tua connessione";
    send(clientdescriptor, &msg, sizeof(msg), 0);

    //Qui dobbiamo calcolare parametri come pdu, cogwind, ecc...
}


void extract_info(int clientdescriptor) {
    char * msg = "Verranno estratti le informazioni della tua connessione";
    send(clientdescriptor, &msg, sizeof(msg), 0);

    //Qui dobbiamo inviare al client i parametri tipo ip, porta, velocità, ecc...
}


void simulation(int clientdescriptor){

    //privvisorio
    int ip, v, conn;

    char * msg = "Simuleremo i parametri di una possibile connessione";
    send(clientdescriptor, &msg, sizeof(msg), 0);

    msg = "Inserire il possibile ip da utilizzare";
    send(clientdescriptor, &msg, sizeof(msg), 0);
    recv(clientdescriptor, &ip, sizeof(ip), 0);

    msg = "Inserire la velocità della connessione";
    send(clientdescriptor, &msg, sizeof(msg), 0);
    recv(clientdescriptor, &v, sizeof(v), 0);

    msg = "Inserire la tipologia di connessione che si sta utilizzando";
    send(clientdescriptor, &msg, sizeof(msg), 0);
    recv(clientdescriptor, &conn, sizeof(conn), 0);

    //ecc..

    //poi creeremo un altra funzione extracr param che con i parametri della simulazione darà pdu, cogwind, ecc... della connesione simulata
}


void die(char *error) {
    fprintf(stderr, "%s.\n", error);
    exit(1);
}