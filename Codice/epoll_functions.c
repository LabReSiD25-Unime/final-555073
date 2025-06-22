#include "header.h"

//Inizializza un file descriptor epoll e lo restituisce
int epoll_initialize(){
    int epoll_fd;

    //Crea il file descriptor di epoll
    if((epoll_fd = epoll_create1(0)) == -1){
        perror("epoll_create1 failed");
        exit(EXIT_FAILURE);
    }

    return epoll_fd;
}

//Aggiunge un socket alla lista di socket monitorati da epoll
int epoll_add(int socket_fd, int epoll_fd, int et){
    struct epoll_event event;

    //Prepara i parametri per poi aggiungere il socket principale ad epoll
    if(et)
        event.events = EPOLLIN | EPOLLET;
    else
        event.events = EPOLLIN;
    event.data.fd = socket_fd;

    //Aggiunge il socket ad epoll
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1){
        perror("epoll_ctl ADD failed");
        return 1;
    }
    return 0;
}

//Rimuove un socket dalla lista di socket monitorati da epoll
void epoll_remove(int epoll_fd, int fd){
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
   //printf("Client disconnesso\n");
}
