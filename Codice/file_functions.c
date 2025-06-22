#include "header.h"

//Restituisce il file descriptor del file indicato dal parametro path
int open_file(char path[], char method[]){
    int file_fd;
    char *found;
    //I file di configurazione possono essere aperti solo in lettura
    if((found = strstr(path,"configuration/")) == NULL){
        if(!strcmp(method, "GET")){
            //Apre il file descriptor del file che va letto
            file_fd = open(path, O_RDONLY, 0644);
        }
        else if(!strcmp(method,"POST")){
            //Apre il file descriptor in modalità append
            file_fd = open(path, O_RDWR | O_APPEND | O_CREAT, 0644);
        }
        else if(!strcmp(method,"PUT")){
            //Apre il file descriptor del file che va sovrascritto o creato
            file_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
        else if(!strcmp(method,"DELETE")){
            //Apre il file descriptor del file che va cancellato
            file_fd = open(path, O_RDWR, 0644);
        }
    }
    else{
        //Apre il file descriptor del file di configurazione che va letto
        file_fd = open(path, O_RDONLY, 0644);
    }

    return file_fd;
}

//Acquisice il lock su un file
int lock_file(struct flock *lock, int fd){
    lock->l_whence = SEEK_SET;
    lock->l_start = 0;
    lock->l_len = 0;

    //Effettua il lock in lettura di un file
    //F_SETLKW specifica che se il file è già sottoposto ad un lock invece di restituire errore il thread deve solo aspettare
    if(fcntl(fd, F_SETLKW ,lock) == -1){
       //printf("\n\nCan't lock the file on write\n\n");
        close(fd);
        return 1;
    }
     
    return 0;
}

//Rilascia il lock su un file
int unlock_file(struct flock *lock, int fd){
    lock->l_type = F_UNLCK;
    int ret = (fcntl(fd, F_SETLKW ,lock) == -1);
    //if(ret)
       //printf("\n\nCan't unlock the file\n\n");
    close(fd);
    return ret;
}

//Acquisisce il lock esclusivo su un file, vi scrive e poi rilascia il lock
int write_with_lock(char path[], request *req, response *res){
    //Apre il fd del file
    int file_fd = open_file(path, req->method);
    //In caso di errore restituisce un messaggio di errore
    if(file_fd < 0){
       //printf("\n\nCan't open the file %s\n\n", path);
        res->status_code = 404;
        return 1;
    }

    //Struttura per effettuare il lock di un file
    struct flock file_lock;
    file_lock.l_type = F_WRLCK;
    //Effettua il lock esclusivo sul file, in caso di errore restituisce un messaggio di errore
    if(lock_file(&file_lock, file_fd)){
        res->status_code = 500;
        return 1;
    }

    //Scrive sul file
    write(file_fd, req->content, req->content_length);

    //Sincronizza il file con le modifiche effettuate sul file descriptor
    fsync(file_fd);

    //Effettua l'unlock del file
    if(unlock_file(&file_lock, file_fd)){
        res->status_code = 500;
        return 1;
    }

    return 0;
}

//Acquisisce il lock non esclusivo su un file, lo legge e rilascia il lock
int read_with_lock(char path[], request *req, response *res){
    //Apre il fd del file
    int file_fd = open_file(path, req->method);

    //In caso di errore restituisce un messaggio di errore
    if(file_fd < 0){
        //printf("\n\nCan't open the file %s\n\n", path);
        res->status_code = 404;
        return 1;
    }

    //Struttura per effettuare il lock di un file
    struct flock file_lock;
    file_lock.l_type = F_RDLCK;
    //Effettua il lock non esclusivo sul file, in caso di errore restituisce un messaggio di errore
    if(lock_file(&file_lock, file_fd)){
        res->status_code = 500;
        return 1;
    }

    //Assegna a content_length il numero di byte del file
    res->content_length = lseek(file_fd, 0, SEEK_END);
    lseek(file_fd, 0, SEEK_SET);

    //Alloca memoria per il contenuto della risposta
    res->content = malloc(res->content_length+1);

    //In caso di errore restituisce un messaggio di errore
    if(res->content == NULL){
        //printf("\n\nNot Enough Memory\n\n");
        res->status_code = 500;
        file_lock.l_type = F_UNLCK;
        fcntl(file_fd, F_SETLKW ,&file_lock);
        close(file_fd);
        return 1;
    }

    //Legge dal file
    read(file_fd, res->content, res->content_length);
    res->content[res->content_length] ='\0';

    //Effettua l'unlock del file
    if(unlock_file(&file_lock, file_fd)){
        res->status_code = 500;
        return 1;
    }
    
    return 0;
}

//Scrive in append su un file per poi leggere lo stesso file
int write_read_with_lock(char path[], request *req, response *res){
    //Scrive sul file
    if(write_with_lock(path, req, res)){
        return 1;
    }
    //Legge dal file
    return read_with_lock(path,req,res);
}

//Acquisisce il lock esclusivo su un file, effettua un unlink() sul file e rilascia il lock
//QUesti tipo di cancellazione è asincrona in quanto il file verrà cancellato quando tutti i file descriptor ad esso associati verranno chiusi
int unlink_with_lock(char path[], request *req, response * res){
    //Apre il fd del file
    int file_fd = open_file(path, req->method);

    //In caso di errore restituisce un messaggio di errore
    if(file_fd < 0){
       //printf("\n\nCan't open the file %s\n\n", path);
        res->status_code = 404;
        return 1;
    }

    //Struttura per effettuare il lock di un file
    struct flock file_lock;
    file_lock.l_type = F_WRLCK;
    //Effettua il lock esclusivo sul file, in caso di errore restituisce un messaggio di errore
    if(lock_file(&file_lock, file_fd)){
        res->status_code = 500;
        return 1;
    }

    //Cancella il file, in maniera asincrona
    if(!access(path, F_OK)){
        unlink(path);
    }

    //Effettua l'unlock del file
    if(unlock_file(&file_lock, file_fd)){
        res->status_code = 500;
        return 1;
    }
    
    return 0;
}
