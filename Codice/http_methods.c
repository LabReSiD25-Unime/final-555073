#include "header.h"

//Costruisce, a partire dall'URI, il vero percorso del file
int build_path(char uri[], char path[]){
    if (strstr(uri, "..") != NULL || strstr(uri, "//") != NULL) {
        return 1;
    }
    else{
        char tmp[MAX_URI_LEN + strlen("files") + strlen("index.html") + 1];
        strcpy(tmp,path);
        snprintf(path, MAX_URI_LEN + strlen("files") + strlen("index.html") + 1, "%s%s",tmp,uri);
        if(path[strlen(path)-1] == '/'){
            strcpy(tmp,path);
            snprintf(path, MAX_URI_LEN + strlen("files") + strlen("index.html") + 1, "%sindex.html",tmp);
        }
        return 0;
    }
}

//Impedisce alle istruzioni diverse da GET di accedere a index.html
int check_index(char path[], response *res){
    if(!strcmp(path,"files/index.html")){
        res->status_code = 405;
        strcpy(res->allow,"GET");
        return 1;
    }
    return 0;
}

//Impedisce a istruzioni prive di authorizzazione di accedere alla cartella private
int is_authorized(char path[], request *req, response *res){
    int authorized = 0;

    char *found;
    if((found = strstr(path,"/private/")) != NULL){
        sprintf(res->WWW_Authenticate, "private");

        if(strcmp(req->auth_scheme,AUTH_SCHEME) != 0){
            res->status_code = 401;
            //printf("\n\nRicevuto: %s\nRichiesto: %s\n\n", req->auth_scheme, AUTH_SCHEME);
            return authorized;
        }

        if(!read_with_lock("configuration/config.txt", req, res)){
            if(!strcmp(req->authorization, res->content)){
                authorized = 1;
            }
            else{
                //printf("\n\nRicevuto: %s\nRichiesto: %s\n\n", req->authorization, res->content);
                res->status_code = 401;
            }
            free(res->content);
        }
    }
    else{
        authorized = 1;
    }

    return authorized;
}

//Implementa l'istruzione GET, leggendo il contenuto di un file richiesto
int get(char path[], request *req, response *res){

    //Imposta il valore standard della risposta
    res->status_code = 200;

    return read_with_lock(path, req, res);
}

//Implementa l'istruzione POST, scrivendo in append su un file richiesto e leggendo poi lo stesso file
int post(char path[], request *req, response *res){

    //Effettua un controllo per evitare di modificare l'index
    if(check_index(path,res)){
        return 1;
    }

    if(!access(path, F_OK)){
        //Se il file esiste viene impostato lo status-code 200 (OK)
        res->status_code = 200;
    }
    else{
        //Imposta l'URI della risorsa che verrà creata
        strcpy(res->location,req->uri);
        //Altrimenti viene impostato lo status-code 201 (Created)
        res->status_code = 201;
    }

    return write_read_with_lock(path, req, res);
}

//Implementa l'istruzione PUT, creando o sovrascrivendo un file richiesto
int put(char path[], request *req, response *res){

    //Effettua un controllo per evitare di sovrascrivere l'index
    if(check_index(path,res)){
        return 1;
    }

    //Imposta l'URI della risorsa che verrà creata
    strcpy(res->location,req->uri);

    //Imposta il valore standard della risposta
    if(!access(path, F_OK)){
        //Se il file esiste viene impostato lo status-code 204 (No Content)
        res->status_code = 204;
    }
    else{
        //Altrimenti viene impostato lo status-code 201 (Created)
        res->status_code = 201;
    }

    res->content_length = 0;
    res->content = NULL;

    return write_with_lock(path, req, res);
}

int delete(char path[], request *req, response *res){

    //Effettua un controllo per evitare di cancellare l'index
    if(check_index(path,res)){
        return 1;
    }

    //Imposta il valore standard della risposta
    res->status_code = 202;

    return unlink_with_lock(path, req, res);
}

//Elabora le richieste in base al loro metodo
int elaborate_request(request *req, response *res){
    char path[MAX_URI_LEN + strlen("files") + strlen("index.html") + 1];
    sprintf(path,"files");

    //Completa il percorso del file richiesto
    if(build_path(req->uri, path)){
        res->status_code = 404;
        return 1;
    }

    //Se l'azione non è autorizzata non viene compiuta
    if(!is_authorized(path,req,res)){
        return 1;
    }

    if(!strcmp(req->method,"GET")){
        return get(path, req, res);
    }
    else if(!strcmp(req->method,"POST")){
        return post(path, req, res);
    }
    else if(!strcmp(req->method,"PUT")){
        return put(path, req, res);
    }
    else if(!strcmp(req->method,"DELETE")){
        return delete(path, req, res);
    }

    res->status_code = 400;
    return 1;
}
