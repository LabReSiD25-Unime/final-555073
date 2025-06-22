#include "header.h"

//Indirizzo per il server
struct sockaddr_in addr;

//Dimensione della struttura che contiene un indirizzo
socklen_t addr_len = sizeof(addr);

int main(){
    //Socket del server
    int master_fd = listening_socket(&addr, addr_len);

    //Conterrà il file descriptor di epoll
    int epoll_fd = epoll_initialize();

    //Conterrà gli eventi gestiti da epoll
    struct epoll_event events[MAX_EVENTS];

    //Aggiunge master_fd ai socket monitorati da epoll
    if(epoll_add(master_fd, epoll_fd, 0)){
        close(master_fd);
        exit(EXIT_FAILURE);
    }
    
    //Conterrà il tempo limite di attesa
    int timeout;

    //Conterrà il numero di eventi verificati
    int num_events = 0;
    while (1){
        //Definisce il tempo limite
        timeout = 60000;

        //Aspetta che vi siano eventi sui socket monitorati
        num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout);

        //Controlla se vi sono nuovi eventi
        if(num_events > 0){
            //Itera per tutti gli eventi
            for (int i = 0; i < num_events; i++) {
                //Controlla se l'evento è un evento di input
                if(events[i].events & EPOLLIN){
                    //Se l'evento di scrittura è sul master_fd, un client sta provando a connettersi
                    if(events[i].data.fd == master_fd){
                        thread_manager(epoll_fd, events[i].data.fd, (void *) acceptance_thread);
                    }
                    //Altrimenti vi è una richiesta in arrivo
                    else{
                        thread_manager(epoll_fd, events[i].data.fd, (void *) task_thread);
                    }
                }
            }
        }
        //Se il numero di eventi è negativo si è verificato un errore
        else if (num_events < 0){
            perror("epoll_wait failed");
            close(master_fd);
            break;
        }
    }
}
