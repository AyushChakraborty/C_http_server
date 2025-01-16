#include <stdio.h>
#include <sys/socket.h>    //for socket related functions like socket(), bind(), listen(), accept()
#include <stdlib.h>
#include <netinet/in.h>    //for internet specific address family and structures like sockaddr_in
#include <regex.h>         //for regex_t and regex functions
#include <fcntl.h>         //for file control options like O_RDONLY, stands for file control

#define PORT 8080
#define BUFFER_SIZE 104857600  //1MB buffer

int server_fd;                     //file descriptor for server socket, this uniquely identifies the server socket

struct sockaddr_in server_addr;    //struct which contains the server address info
//sockaddr_in is for IPv4, sockaddr_in6 is for IPv6


void build_http_response(char *filename, const char *file_ext, char *response, size_t *response_len) {
    //build http header
    const char *mime_type = get_mime_type(file_ext);   //gets the mime of the file based on its extention
    char *header = (char *)malloc(sizeof(char) * BUFFER_SIZE);
    snprintf(header, BUFFER_SIZE, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "\r\n", mime_type);
    
    //try to open the file requested by the client
    int file_fd = open(filename, O_RDONLY);   //this flag opens the file in read only mode
    //if file not found
    if (file_fd == -1) {
        snprintf(response, BUFFER_SIZE, 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "404 Not Found"
                );
        *response_len = strlen(response);
        return;
    }

    //get file size for content length
    //....

}


void handle_client(void *arg) {
    //point of this func is to handle the comm with the client in the thread that gets creates
    //by the server socket when the client wants to send a req to the server
    int client_fd = *((int *)arg);
    char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));   //allocating memory for the buffer


    //recieve the req data from the client and store it in the buffer
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0) {
        //see if req is GET
        regex_t regex;    //is used to hold the compiled regular exp, which is later
        //used to match against the http req received from the client 
        regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);   //the compiled format is then stored in regex var
        //the format we are looking for is of type say "GET /index.html HTTP/1.1"
        //also REG_EXTENDED flag modifies how the regex is interpreted, with this flag, POSIX extended regex is used
        //which allows more modern, flexible syntax while specifying the regex pattern to be compiled
        regmatch_t matches[2];

        if (regexec(&regex, buffer, 2, matches, 0) == 0) {   //if the req contents in the buffer matches that in
        //the pattern compiled into regex variable, then do the following

            //extract the filename from req and decode url
            buffer[matches[1].rm_eo] = '\0';   //setting the end offset of the captured group to be null byte,
            //essentially isolating this substring in buffer
            const char *url_encoded_filename = buffer + matches[1].rm_so*sizeof(char);    //moves the pointer to the start
            //of the matched substring, as buffer itself is a pointer to the actual buffer object
            char *file_name = url_decode(url_encoded_filename);    //TODO


            //get file extension
            char file_ext[32];    //copy the file extension obtained from the get_file_extension(file_name) func
            //to file_ext variable made here
            strcpy(file_ext, get_file_extension(file_name));       //TODO


            //build http response
            char *response = (char *)malloc(BUFFER_SIZE * 2 * sizeof(char));
            size_t response_len;
            build_http_response(file_name, file_ext, response, &response_len);


            //send http res to client 
            send(client_fd, response, response_len, 0);

            free(response);
            free(file_name);
        }
        regfree(&regex);
    }
    close(client_fd);
    free(arg);
    free(buffer);
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
    //connections on any of the machines' available network interfaces like ethernet, WIFI
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

