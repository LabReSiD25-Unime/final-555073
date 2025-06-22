CC = gcc
CFLAGS = -g -Wall
TARGET = server_HTTP
OBJS = server.o thread_functions.o http_methods.o sending_functions.o parsing_functions.o file_functions.o epoll_functions.o socket_functions.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

check:
	@echo "\n### Simboli server ###"
	nm server.o
	@echo "\n### Simboli thread:functions ###"
	nm thread_functions.o
	@echo "\n### Simboli http_methods ###"
	nm http_methods.o
	@echo "\n### Simboli sending_functions ###"
	nm sending_functions.o
	@echo "\n### Simboli parsing_functions ###"
	nm parsing_functions.o
	@echo "\n### Simboli file_functions ###"
	nm file_functions.o
	@echo "\n### Simboli epoll_functions ###"
	nm epoll_functions.o
	@echo "\n### Simboli socket_functions ###"
	nm socket_functions.o

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean check
