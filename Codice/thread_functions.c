#include "header.h"

extern struct sockaddr_in addr;
extern socklen_t addr_len;

//Mutex che regola l'accettazione di un nuovo client
pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;

//Thread che gestisce le richieste in ingresso
void *task_thread(event_t *arg){
    //Buffer che riceverà la richiesta in input
    char buffer[BUFFER_SIZE];

    //Strutture per le richieste e le risposte
    response res;
    request req;

    //Riceve la richiesta
    int n;
    while((n = recv(arg->socket_fd, buffer, BUFFER_SIZE - 1, 0)) > 0){
        //Se riceve n byte elaboriamo la richiesta
        buffer[n] = '\0';
        char *message = buffer;
        //printf("Dati disponibili in lettura\n");
        while(*message != '\0'){
            //printf("Socket %d = \n------------------------------\n%s\n------------------------------\n", arg->socket_fd, message);
            //Effettua il parsing della richiesta
            if(!parse(&message, &req, &res)){
                //Se la richiesta non presenta errori di sintassi viene elaborata
                elaborate_request(&req, &res);
            }
            //Invia una risposta al client
            http_send(&res, arg->socket_fd);
            //Se è necessario, chiude la connessione
            if((strcmp(res.connection,"close") == 0) || (strcmp(req.connection,"close") == 0)){
                epoll_remove(arg->epoll_fd,arg->socket_fd);
                free(arg);
                pthread_exit(NULL);
            }
        }
    }
    //se n = 0 allora un utente si è disconnesso
    if(n == 0){
        epoll_remove(arg->epoll_fd,arg->socket_fd);
    }
    else if(errno != EWOULDBLOCK && errno != EAGAIN){
        epoll_remove(arg->epoll_fd,arg->socket_fd);
    }
    //Libera le risorse non più utili
    free(arg);
    pthread_exit(NULL);
}

//Thread che accetta le connessioni in ingresso
void *acceptance_thread(event_t *arg){

    //Acquisisce un mutex per evitare race conditions su addr e addrlen
    pthread_mutex_lock(&accept_mutex);
        int new_socket;

        //Accetta la nuova connessione e l'aggiunge alla lista di connessioni monitorate da epoll
        if((new_socket = socket_accept(arg->socket_fd, (struct sockaddr *) &addr, &addr_len)) > 0){
            if(epoll_add(new_socket, arg->epoll_fd, 1)){
                close(new_socket);
            }
        }

        //Libera le risorse utilizzate
        free(arg);
    pthread_mutex_unlock(&accept_mutex);
    pthread_exit(NULL);
}

//Gestisce la creazione dei threads
void thread_manager(int epoll_fd, int socket_fd, void *function){
    pthread_t thread;

    //Struttura che sarà usata come argomento dei threads
    event_t *arg = malloc(sizeof(event_t));
    arg->epoll_fd = epoll_fd;
    arg->socket_fd = socket_fd;

    //Crea il thread e lo scollega dal thread principale
    pthread_create(&thread, NULL, (void *) function, arg);
    pthread_detach(thread);
}
