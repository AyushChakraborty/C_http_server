#include <stdio.h>
#include <sys/socket.h>    //for socket related functions like socket(), bind(), listen(), accept()
#include <stdlib.h>
#include <netinet/in.h>    //for internet specific address family and structures like sockaddr_in

#define PORT 8080

int server_fd;                     //file descriptor for server socket, this uniquely identifies the server socket

struct sockaddr_in server_addr;    //struct which contains the server address info
//sockaddr_in is for IPv4, sockaddr_in6 is for IPv6

void handle_client(void *arg) {
    //point of this func is to handle the comm with the client in the thread that gets creates
    //by the server socket with the client when the client wants to send a req to the server
}

int main() {
    //create server socket, AF_INET specifies the Address Family IPv4 in this case, 
    //SOCK_STREAM specifies the type of socket, in this case TCP which is a protocol
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE); 
    }

    //configuring the socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  //INADDR_ANY allows the server to accept
    //connections on any of the machines available network interfaces like ethernet, WIFI
    server_addr.sin_port = htons(PORT);       //htons() converts the port number to network byte order
    //which is just big endian representation of the port number

    //bind socket to port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
        //the bind func binds the server socket to the port number and the IP address
        //here typecasting is done as bind expects a generic pointer to a sockaddr struct
        //not some specific IPv4 or IPv6 struct
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    //listen for incoming connections
    if (listen(server_fd, 7) < 0) {   //7 is the max number of pending connections that can be queued
    //before the server starts rejecting new connections, this value is a limit on how 
    //many clients can wait to be served while the server processes other clients
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    //now we handle incoming client connections 
    /*when a client connects to the server(through the socket), the server accepts the connection
    and creates a new thread to handle the clients http req, this way the server can handle multiple
    clients concurrently
    */
   while (1) {
    //client info
    struct sockaddr_in client_addr;  //will store the clients IP address and port number
    // socklen_t client_addr_len = sizeof(client_addr);
    int *client_fd = malloc(sizeof(int));   //this stores the file descriptor for the client socket

    //accept the client conn
    if ((*client_fd = accept(server_fd, (struct sockaddr *)&client_addr, sizeof(client_addr))) < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }     
       //this was handling the client connection, now that its established, a thread for this 
       //client is created by the server to handle the http req and resps
       pthread_t thread_id;
       pthread_create(&thread_id, NULL, handle_client, (void *)client_fd);
       //NULL here means that the thread will be created with default attributes like 
       //default stack sizem scheduling policy etc, and handle_client is the func 
       //which will be executed when the thread is started, its also known as the
       //thread routine/entry point
       //again also typecasted in (void *) as the thread routine expects a void pointer
       //wihtin this API call, client_fd is passed as an argument to the thread routine
       //which is handle_client hence the typecast is needed as handle_client's signature
       //is void *handle_client(void *arg)
       pthread_detach(thread_id); 
   }


    return 0;
}

