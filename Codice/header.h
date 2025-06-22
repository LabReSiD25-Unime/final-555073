#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#define BUFFER_SIZE 10000
#define PORT 8080
#define MAX_EVENTS 1000
#define MAX_METHOD_LEN 7
#define MAX_URI_LEN 2000
#define MAX_REASON_PHRASE_LEN 22
#define MAX_CONNECTION_LEN 11
#define MAX_HEADER_LEN 4000
#define MAX_REALM_LEN 8
#define MAX_ALLOW_LEN 23
#define HTTP_VERSION 1.1
#define AUTH_SCHEME "Basic"
#define MAX_AUTH_LEN 100
#define MAX_AUTH_SCHEME_LEN 7

#define MESSAGE_400 "<html><head><title>400 Bad Request</title></head><body><h1>400 Bad Request</h1><p>Your browser sent a request that this server could not understand.</p></body></html>"
#define MESSAGE_400_LEN 167
#define MESSAGE_401 "<html><head><title>401 Unauthorized</title></head><body><h1>401 Unauthorized</h1><p>Permission is required.</p></body></html>"
#define MESSAGE_401_LEN 126
#define MESSAGE_404 "<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>"
#define MESSAGE_404_LEN 140
#define MESSAGE_414 "<html><head><title>414 Request-URI Too Large</title></head><body><h1>414 Request-URI Too Large</h1><p>The requested URI is too long for this server to process.</p></body></html>"
#define MESSAGE_414_LEN 178
#define MESSAGE_500 "<html><head><title>500 Internal Server Error</title></head><body><h1>500 Internal Server Error</h1><p>An unexpected error occurred.</p></body></html>"
#define MESSAGE_500_LEN 150

#ifndef TYPES
#define TYPES
    typedef struct request{
        char method[MAX_METHOD_LEN];
        char uri[MAX_URI_LEN];
        char connection[MAX_CONNECTION_LEN];
        long int content_length;
        char auth_scheme[MAX_AUTH_SCHEME_LEN];
        char authorization[MAX_AUTH_LEN];
        char *content;
    } request;

    typedef struct response{
        int status_code;
        char reason_phrase[MAX_REASON_PHRASE_LEN];
        long int content_length;
        char location[MAX_URI_LEN];
        char connection[MAX_CONNECTION_LEN];
        char allow[MAX_ALLOW_LEN];
        char WWW_Authenticate[MAX_REALM_LEN];
        char *content;
    } response;

    typedef struct event_t{
        int epoll_fd;
        int socket_fd;
    } event_t;
#endif

//socket_functions.c
int listening_socket(struct sockaddr_in *addr, socklen_t addr_len);
int socket_accept(int server_fd, struct sockaddr * addr, socklen_t * addr_len);

//epoll_functions.c
int epoll_initialize();
int epoll_add(int socket_fd, int epoll_fd, int et);
void epoll_remove(int epoll_fd, int fd);

//file_functions.c
int write_with_lock(char path[], request *req, response *res);
int read_with_lock(char path[], request *req, response *res);
int write_read_with_lock(char path[], request *req, response *res);
int unlink_with_lock(char path[], request *req, response * res);

//parsing_functions.c
int parse(char *message[], request *req, response *res);

//sending_functions.c
void http_send(response *res, int fd);

//http_methods.c
int elaborate_request(request *req, response *res);

//threads_functions.c
void *task_thread(event_t *arg);
void *acceptance_thread(event_t *arg);
void thread_manager(int epoll_fd, int socket_fd, void *function);