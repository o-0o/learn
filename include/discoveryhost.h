#ifndef RECEIVER_H

#include <sys/socket.h>
#include <memory.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <iostream>
#include <vector>

#define PORT 12345
#define ALIVE_TIME 5

#define NEW_DEVICE 1
#define NEW_ADDRESS 2
#define DEVICE_NOT_ALIVE 3
#define ADDRESS_NOT_ALIVE 4

typedef struct Message {
    uint64_t host_id;
    std::vector<char *> ips;
    in_port_t image_port;
    in_port_t control_port;
} Message;

typedef struct {
    time_t scantime;
    struct sockaddr_in address;
} Address;

typedef struct {
    Message msg; 
    std::vector<Address> addresses;
} Dev;

typedef void (*Callback)(void *,int,void *);

typedef struct {
    int sock;
    in_port_t port;
    time_t sender_timer;
    bool running;
    time_t interval=1;
    time_t recv_timeout=1;
    Message message;
    struct sockaddr_in broadcast_addr;
    std::vector<Dev> devs;
    std::vector<Callback> callbacks;
    pthread_t receiver_thread;
} Host;

void *scan(void *arg);
void run_callbacks(Host *host,int type,void *arg,void *msg);
void discovery_scan_device_loop(Host *host);
int  discovery_initHost(Host* host,in_port_t port);
void discovery_scan_device(Host *host);
void checkAlive(Host *host);
void *receiver(void *arg);
void register_callback(Host *host,Callback callback);

#endif