### simple http server written in C, handles multi thread client connections like a standard server, all the theoretical information about the topic is written in notes.txt, kindly refer to it

#How to start the server</span>

### 1) get the server script in your machine(http_server.c)
### 2) compile the server script using 
```sh
gcc -o server http_server.c
```
### 3) execute the server executable using 
```sh
./server
```
### 4) go to localost:8080(the port where the server script is available at 8080)
### 5) you will be able to see 404 Not Found, then given you have file(like a html file) you want to be displayed in the browser, do the following <span style="color:yellow">localhost:8080/file.extension</span>
### 6) you will be able to see the debug logs like the name of the file before, after the url decoding, and can also see the file being displayed on the browser, yay :)