// tcp_client.c
/*This file contains the TCP client implementation
*AUTHOR - Aysvarya Gopinath
*REFERENCE-https://www.geeksforgeeks.org/socket-programming-cc/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9000
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr; //server address struct
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    // Set server info
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Connect to localhost (127.0.0.1) //rpi wifi 4--10.70.2.219   //ethernet--128.138.189.226
    if (inet_pton(AF_INET, "128.138.189.226", &serv_addr.sin_addr) <= 0) {  //sets the IP address of the server in the struct
        perror("Invalid address");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    // Receive metrics from server
    int bytes_received;
    while ((bytes_received = read(sock, buffer, BUFFER_SIZE - 1)) > 0) { //reads data from server into buffer
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    close(sock);
    return 0;
}



























