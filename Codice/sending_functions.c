#include "header.h"

//Completa i messaggi HTTP di risposta in base al loro status-code
void resolve_status_code(response *res){
    switch (res->status_code){
        case 200:
            strcpy(res->reason_phrase,"OK");
            strcpy(res->connection, "keep-alive");
            break;
        case 201:
            strcpy(res->reason_phrase,"Created");
            strcpy(res->connection, "keep-alive");
            break;
        case 202:
            strcpy(res->reason_phrase,"Accepted");
            strcpy(res->connection, "keep-alive");
            res->content_length = 0;
            res->content = NULL;
            break;
        case 204:
            strcpy(res->reason_phrase,"No Content");
            strcpy(res->connection, "keep-alive");
            res->content = NULL;
            break;
        case 400:
            strcpy(res->reason_phrase,"Bad Request");
            strcpy(res->connection, "close");
            res->content_length = MESSAGE_400_LEN;
            res->content = malloc(MESSAGE_400_LEN);
            strcpy(res->content, MESSAGE_400);
            break;
        case 401:
            strcpy(res->reason_phrase,"Unauthorized");
            strcpy(res->connection, "keep-alive");
            res->content_length = MESSAGE_401_LEN;
            res->content = malloc(MESSAGE_401_LEN);
            strcpy(res->content, MESSAGE_401);
            break;
        case 404:
            strcpy(res->reason_phrase,"Not Found");
            strcpy(res->connection, "keep-alive");
            res->content_length = MESSAGE_404_LEN;
            res->content = malloc(MESSAGE_404_LEN);
            strcpy(res->content, MESSAGE_404);
            break;
        case 405:
            strcpy(res->reason_phrase,"Method Not Allowed");
            strcpy(res->connection, "keep-alive");
            res->content_length = 0;
            res->content = NULL;
            break;
        case 414:
            strcpy(res->reason_phrase,"Request-URI Too Large");
            strcpy(res->connection, "close");
            res->content_length = MESSAGE_414_LEN;
            res->content = malloc(MESSAGE_414_LEN);
            strcpy(res->content, MESSAGE_414);
            break;
        case 500:
            strcpy(res->reason_phrase,"Internal Server Error");
            strcpy(res->connection, "close");
            res->content_length = MESSAGE_500_LEN;
            res->content = malloc(MESSAGE_500_LEN);
            strcpy(res->content, MESSAGE_500);
            break;
        case 501:
            strcpy(res->reason_phrase,"Not Implemented");
            strcpy(res->connection,"keep-alive");
            res->content_length = 0;
            res->content = NULL;
            break;
    }
}

//Invia la il messaggio HTTP di risposta al client
void http_send(response *res, int fd){
    //Alloca la memoria necessaria a contenere il messaggio
    char *message = malloc(MAX_HEADER_LEN + (res->content ? res->content_length : 0 ));

    //Se non vi è abbastanza memoria, allochiamo della memoria minore per inviare un messaggio di errore
    if(message == NULL){
        message = malloc(MESSAGE_500_LEN);
       //printf("\n\nNOT ENOUGH MEMORY\n\n");
        res->status_code = 500;
    }

    //Completa il messaggio HTTP di risposta
    resolve_status_code(res);

    //Formatta il messaggio HTTP di risposta
    switch(res->status_code){
        case 201:
            sprintf(message, "HTTP/%1.1f %d %s\r\nConnection: %s\r\nContent-Length: %ld\r\nLocation: %s\r\n\r\n%s", HTTP_VERSION, res->status_code, res->reason_phrase, res->connection, res->content_length, res->location, (res->content ? res->content : "\0"));
            break;
        case 204:
            sprintf(message, "HTTP/%1.1f %d %s\r\nConnection: %s\r\nLocation: %s\r\n\r\n", HTTP_VERSION, res->status_code, res->reason_phrase, res->connection, res->location);
            break;
        case 202:
            sprintf(message, "HTTP/%1.1f %d %s\r\nConnection: %s\r\nContent-Length: %ld\r\n\r\n", HTTP_VERSION, res->status_code, res->reason_phrase, res->connection, res->content_length);
            break;
        case 501:
            sprintf(message, "HTTP/%1.1f %d %s\r\nConnection: %s\r\nContent-Length: %ld\r\n\r\n", HTTP_VERSION, res->status_code, res->reason_phrase, res->connection, res->content_length);
            break;
        case 401:
            sprintf(message, "HTTP/%1.1f %d %s\r\nConnection: %s\r\nWWW-Authenticate: %s realm=%c%s%c\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n%s\r\n\r\n", HTTP_VERSION, res->status_code, res->reason_phrase, res->connection, AUTH_SCHEME, '"', res->WWW_Authenticate, '"',res->content_length, res->content);
            break;
        case 405:
            sprintf(message, "HTTP/%1.1f %d %s\r\nConnection: %s\r\nContent-Length: %ld\r\nAllow: %s\r\n\r\n", HTTP_VERSION, res->status_code, res->reason_phrase, res->connection, res->content_length, res->allow);
            break;
        default:
            sprintf(message, "HTTP/%1.1f %d %s\r\nConnection: %s\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n%s", HTTP_VERSION, res->status_code, res->reason_phrase, res->connection, res->content_length, res->content);
    }

    //printf("%s\n",message);

    //Invia il messaggio HTTP di risposta
    send(fd, message, strlen(message), 0);

    //Libera le risorse non più utili
    free(message);
    if(res->content != NULL){
        free(res->content);
    }
}
