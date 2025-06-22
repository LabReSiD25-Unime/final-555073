#include "header.h"

//Copia il metodo della richiesta dal messaggio alla struttura request
int get_method(char message[], request *req, response *res){
    int i = 0;
    //Copia il metodo dal messaggio alla struttura request
    while(message[i] != ' '){
        if(i >= MAX_METHOD_LEN){
            res->status_code = 400;
            return -1;
        }
        req->method[i] = message[i];
        i++;
    }
    req->method[i] = '\0';
    i++;

    //Controlla che il metodo sia un metodo valido
    if((strcmp(req->method,"GET") != 0) && (strcmp(req->method,"POST") != 0) && (strcmp(req->method,"PUT") != 0) && (strcmp(req->method,"DELETE") != 0)){
        res->status_code = 400;
        return -1;
    }

    return i;
}

//Copia l'URI dal messaggio alla struttura request
int get_uri(char message[], int i, request *req, response *res){
    int j = 0;
    while(message[i] != ' '){
        if(j >= MAX_URI_LEN){
            res->status_code = 414;
            return 1;
        }
        req->uri[j] = message[i];
        i++;
        j++;
    }
    req->uri[j] = '\0';

    if(strlen(req->uri) == 0){
        res->status_code = 400;
        return 1;
    }

    return 0;
}

//Verifica la presenza dei caratteri di fine linea obbligatori nei pacchetti HTTP
int find_newline(char message[]){
    char *found;

    if((found = strstr(message, "\r\n")) == NULL){
        return 1;
    }

    return 0;
}

//Verifica la presenza del campo HOST
int find_host(char message[]){
    char *found;
    //La presenza dell'Host è obbligatoria secondo l'RFC2616
    if((found = strstr(message, "Host:")) == NULL){
        return 1;
    }

    return 0;
}

//Copia il campo Connection dal messaggio alla struttura request
int get_connection(char message[], request *req){
    char *found;
    int i;

    //Verifica la presenza di Connection
    found = strstr(message, "Connection:");
    //Se presente viene copiato dal messaggio alla struttura request
    if(found != NULL){
        found += strlen("Connection:");
        while(found[0] == ' ' || found[0] == '\t'){
            found++;
        }
        i = 0;
        while(found[i] != '\r' && found[i] != ' ' && found[i] != '\n'){
            req->connection[i] = found[i];
            i++;
        }
        req->connection[i] = '\0';

        //Connection può essere solo "keep-alive" oppure "close"
        if((strcmp(req->connection,"keep-alive") != 0) && (strcmp(req->connection,"close") != 0)){
            return 1;
        }
    }
    //Altrimenti il valore di default è "keep-alive"
    else{
        strcpy(req->connection,"keep-alive");
    }

    return 0;
}

//Copia il body dal messaggio alla struttura request
int get_body(char message[], request *req){
    char *found;
    //Cerca il campo Content-Length all'interno del messaggio
    found = strstr(message, "Content-Length:");

    //Se non presente, viene restituito un valore di errore
    if(found == NULL){
        return 1;
    }
    //In caso contrario viene convertito in intero e salvato nella struttura request
    found += strlen("Content-Length:");
    req->content_length = atoi(found);
    //Se minore o uguale a 0 viene restituito un valore di errore
    if(req->content_length <= 0){
        return 1;
    }

    //Cerca la fine dell'header
    found = strstr(message, "\r\n\r\n");

    //Se non presente viene restituito un messaggio di errore
    if(found == NULL){
        return 1;
    }
    //Altrimenti viene copiato il contenuto del messaggio dopo l'header nella struttura request
    found += strlen("\r\n\r\n");
    req->content = malloc(req->content_length);
    for(int i = 0; i < req->content_length; i++){
        req->content[i] = found[i];
    }

    return 0;
}

//Copia il campo Authorization dal messaggio alla struttura request
int get_authorization(char message[], request *req){
    char *found;

    //Cerca il campo Authorization all'interno del messaggio
    if((found = strstr(message, "Authorization:")) !=  NULL){
        found += strlen("Authorization:");
        while(*found == ' ' || *found == '\t'){
            found++;
        }
        int i = 0;
        //Se presente, lo schema di crittografia viene copiato nel campo auth_scheme della struttura request
        while(found[i] != '\r' && found[i] != ' ' && found[i] != '\n'){
            if(i >= MAX_AUTH_LEN){
                return 1;
            }
            req->auth_scheme[i] = found[i];
            i++;
        }
        req->auth_scheme[i] = '\0';

        found += strlen(req->auth_scheme);
        while(*found == ' ' || *found == '\t'){
            found++;
        }

        i = 0;
        //Vengono copiate le credenziali dal messaggio al campo authorization della struttura request
        while(found[i] != '\r' && found[i] != ' ' && found[i] != '\n'){
            if(i >= MAX_AUTH_LEN){
                return 1;
            }
            req->authorization[i] = found[i];
            i++;
        }
        req->authorization[i] = '\0';
    }
    else{
        req->auth_scheme[0] = '\0';
        req->authorization[0] = '\0';
    }

    return 0;
}

//Effettua il parsing del messaggio
int parse(char **message, request *req, response *res){

    //Estrae il metodo dal messaggio
    int i = get_method(*message,req,res);
    if(i == -1){
        return 1;
    }

    //Estrae l'URI dal messaggio
    if(get_uri(*message,i,req,res)){
        return 1;
    }

    //Cerca almeno un CRLF, un header Host e tenta di estrarre un header Connection
    if(find_newline(*message) || find_host(*message) || get_connection(*message,req)){
        res->status_code = 400;
        return 1;
    }

    char *found;
    //Cerca la riga vuota che termina l'head della richiesta
    found = strstr(*message, "\r\n\r\n");
    //In caso di errore restituisce un valore di errore e imposta il codice di stato della risposta
    if(found == NULL){
        res->status_code = 400;
        return 1;
    }
    found += strlen("\r\n\r\n");

    //I metodi POST e PUT richiedono di avere un body
    if(req->method[0] == 'P'){
        if(get_body(*message,req)){
            res->status_code = 400;
            return 1;
        }

        *message = found + req->content_length;

        //PUT richiede di non ignorare alcun header del tipo Content-*, se una funzione non è implementata restituisce quindi 501
        if(!strcmp(req->method,"PUT")){
            if((strstr(*message,"Content-Encoding") != NULL) || (strstr(*message,"Content-Location") != NULL) || (strstr(*message,"Content-Range") != NULL)){
                res->status_code = 501;
                return 1;
            }
        }
    }
    else{
        *message = found;
    }

    //Tenta di estrarre dal messaggio l'header Authorization
    if(get_authorization(*message,req)){
        res->status_code = 400;
        return 1;
    }

    return 0;
}
