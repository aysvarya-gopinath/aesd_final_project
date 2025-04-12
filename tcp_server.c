// tcp_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_METRICS 2048

void get_system_metrics(char *buffer) {
    FILE *fp;
    char cmd[256];
    char temp[512];

    buffer[0] = '\0'; // clear buffer

    // CPU usage
    strcat(buffer, "CPU Usage:\n");
    fp = popen("top -bn1 | grep 'Cpu(s)'", "r");
    if (fp) {
        while (fgets(temp, sizeof(temp), fp)) {
            strcat(buffer, temp);
        }
        pclose(fp);
    }

    // Memory usage
    strcat(buffer, "\nMemory Usage:\n");
    fp = popen("free -m", "r");
    if (fp) {
        while (fgets(temp, sizeof(temp), fp)) {
            strcat(buffer, temp);
        }
        pclose(fp);
    }

    // Disk usage
    strcat(buffer, "\nDisk Usage:\n");
    fp = popen("df -h", "r");
    if (fp) {
        while (fgets(temp, sizeof(temp), fp)) {
            strcat(buffer, temp);
        }
        pclose(fp);
    }

    // Uptime
    strcat(buffer, "\nSystem Uptime:\n");
    fp = popen("uptime -p", "r");
    if (fp) {
        while (fgets(temp, sizeof(temp), fp)) {
            strcat(buffer, temp);
        }
        pclose(fp);
    }

    // Running processes
    strcat(buffer, "\nRunning Processes:\n");
    fp = popen("ps -e --no-headers | wc -l", "r");
    if (fp) {
        while (fgets(temp, sizeof(temp), fp)) {
            strcat(buffer, temp);
        }
        pclose(fp);
    }
    
     // Network Activity
    strcat(buffer, "\nNetwork Activity:\n");
    fp = popen("netstat -i", "r");
    if (fp) {
        while (fgets(temp, sizeof(temp), fp)) {
            strcat(buffer, temp);
        }
        pclose(fp);
    }

    // System Event Logs (dmesg)
  /* strcat(buffer, "\nSystem Event Logs:\n");
    fp = popen("dmesg | tail -n 10", "r");  // Fetch last 10 events
    if (fp) {
        while (fgets(temp, sizeof(temp), fp)) {
            strcat(buffer, temp);
        }
        pclose(fp);
    }*/
    
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char metrics[MAX_METRICS];

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket to IP/port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connection
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept client connection
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    // Get and send system metrics
    get_system_metrics(metrics);
    send(new_socket, metrics, strlen(metrics), 0);
    printf("System metrics sent to client.\n");

    close(new_socket);
    close(server_fd);
    return 0;
}

/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char *message = "Hello from server!\n";

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // localhost
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    printf("Server listening on port %d...\n", PORT);

    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (new_socket < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    send(new_socket, message, strlen(message), 0);
    printf("Message sent to client.\n");

    close(new_socket);
    close(server_fd);
    return 0;
}
*/
