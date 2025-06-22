#include "header.h"

//Inizializza un socket e lo restituisce
int socket_initialize(){
    int socket_fd;

    //Crea il socket
    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    return socket_fd;
}

//Inizializza l'indirizzo dato in ingresso ed effettua il binding con il socket
void socket_bind(int socket_fd, struct sockaddr_in *addr, socklen_t addr_len){

    //Configura l'indirizzo del server
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = INADDR_ANY;
    addr->sin_port = htons(PORT);

    //Binding del socket
    if(bind(socket_fd, (struct sockaddr *) addr, addr_len) < 0){
        perror("binding failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
}

//Imposta il socket in ascolto
void socket_listen(int socket_fd){
    if(listen(socket_fd, SOMAXCONN) < 0){
        perror("listening failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
}

//Imposta il socket in modalità non bloccante
void set_nonblocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
}

//Restituisce un socket non bloccante in ascolto
int listening_socket(struct sockaddr_in *addr, socklen_t addr_len){
    int socket_fd = socket_initialize();

    socket_bind(socket_fd, addr, addr_len);

    socket_listen(socket_fd);

    set_nonblocking(socket_fd);

    return socket_fd;
}

//Accetta una connessione in ingresso su un socket
int socket_accept(int server_fd, struct sockaddr * addr, socklen_t * addr_len){
    int new_socket;
    
    if((new_socket = accept(server_fd, addr, addr_len)) == -1){
        if(errno != EWOULDBLOCK && errno != EAGAIN){
            perror("accept failed");
            return -1;
        }
    }

    set_nonblocking(new_socket);

    return new_socket;
}
