#include <socket.h>
#include <string.h>

int main(int argc, char * argv) {
    //Inizializzo la struttura
    struct sockaddr_in local, client;

    local.sin_family = AF_INET;
    local.sin_port = htons(8080);
    local.sin_address.s_addr = INADDR_ANY

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    //faccio il controllo sul socket
    if (serverSocket == -1){
        printf("Errore nella creazione del socket");
        exit(-1);
    } 

    int status = bind(serverSocket, (struct sockaddr * ) & local, sizeof(local));
    //faccio il controllo sulla bind
    if (status == -1){
        printf("Errore nel binding");
        exit(-1);
    }

    listen(sb, 5);

    while (1) {
        int ss, pid;
        ss = accept(serverSocket, (struct sockaddr * ) & client, sizeof(struct sockaddr));

        pid = fork();
        if (pid != 0) {
            //Processo padre: chiudo il socket di servizio
            close(ss);
        } else {
            //Processo figlio: chiudo il socket di benvenuto
            close(serverSocket);

            char scelta[10];
            //Attendo un messaggio dal client
            recv(ss, & scelta, sizeof(scelta), 0)

            if (strcmp(scelta, "parametriConnessione") == 0) {
                extract_param(ss);
            } else if (strcmp(scelta, "infoConnessione") == 0) {
                extract_info(ss);
            } else if(strcmp(scelta, "simulaConnessione") == 0){
                simulation(ss);
            }else {
                char * msg = "Questa funzione non è stata implementata";
                send(ss, &msg, sizeof(msg), 0);
            }

            //Chiudo il socket di servizio ed esco
            close(ss);

            return 0;
        }
    }
}

void extract_param(int ss) {
    char * msg = "Verranno estratti i parametri della tua connessione";
    send(ss, &msg, sizeof(msg), 0);

    //Qui dobbiamo calcolare parametri come pdu, cogwind, ecc...
}

void extract_info(int ss) {
    char * msg = "Verranno estratti le informazioni della tua connessione";
    send(ss, &msg, sizeof(msg), 0);

    //Qui dobbiamo inviare al client i parametri tipo ip, porta, velocità, ecc...
}

void simulation(int ss){
    char * msg = "Simuleremo i parametri di una possibile connessione";
    send(ss, &msg, sizeof(msg), 0);

    msg = "Inserire il possibile ip da utilizzare";
    send(ss, &msg, sizeof(msg), 0);
    recv(ss, &ip, sizeof(ip), 0);

    msg = "Inserire la velocità della connessione";
    send(ss, &msg, sizeof(msg), 0);
    recv(ss, &v, sizeof(v), 0);

    msg = "Inserire la tipologia di connessione che si sta utilizzando";
    send(ss, &msg, sizeof(msg), 0);
    recv(ss, &conn, sizeof(conn), 0);

    //ecc..

    //poi creeremo un altra funzione extracr param che con i parametri della simulazione darà pdu, cogwind, ecc... della connesione simulata
}
