// tcp_server.c
/*FILE-This file contains the tcp server implementation to run the system analysis and send to client
*AUTHOR-Aysvarya Gopinath
*REFERENCES-https://github.com/cu-ecen-aeld/assignments-3-and-later-aysvarya-gopinath/blob/main/server/aesdsocket.c
//https://www.geeksforgeeks.org/socket-programming-cc/
//https://beej.us/guide/bgnet/html/
//https://opensource.com/article/22/4/parsing-data-strtok-c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>


#define PORT 9000
#define MAX_METRICS 4048

void get_system_metrics(char *buffer) {
    FILE *fp;
    char temp[4096];
    char *token;

    buffer[0] = '\0'; // clear buffer

    // CPU usage
    strcat(buffer, "CPU Usage:\n");
          
fp = fopen("/proc/stat", "r");  // open /proc/stat
if (fp) {
    if (fgets(temp, sizeof(temp), fp)) {
        if (strncmp(temp, "cpu ", 4) == 0) { // only match "cpu " line
        strcat(buffer, temp);
            unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
            sscanf(temp, "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                   &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);

            // Print each field separately
            char cpu_info[512];
            snprintf(cpu_info, sizeof(cpu_info),
                     "  User time:     %llu\n"
                     "  Nice time:     %llu\n"
                     "  System time:   %llu\n"
                     "  Idle time:     %llu\n"
                     "  IOwait time:   %llu\n"
                     "  IRQ time:      %llu\n"
                     "  SoftIRQ time:  %llu\n"
                     "  Steal time:    %llu\n"
                     "  Guest time:    %llu\n"
                     "  Guest Nice:    %llu\n",
                     user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice);

            strcat(buffer, cpu_info);

            // Now calculate CPU usage
            unsigned long long idle_time = idle + iowait;
            unsigned long long non_idle = user + nice + system + irq + softirq + steal;
            unsigned long long total = idle_time + non_idle;

            float cpu_usage = (float)(total - idle_time) / total * 100.0;

            snprintf(cpu_info, sizeof(cpu_info), "CPU Usage percent: %.2f%%\n", cpu_usage);
            strcat(buffer, cpu_info); 
          }     
        }
         fclose(fp);
    }   
     

    // Memory usage
    strcat(buffer, "\nMemory Usage:\n");
    fp = popen("free -h", "r");
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
    strcat(buffer, "\nSystem timings:\n");
    fp = popen("uptime", "r"); //uptime -p
    if (fp) {
        while (fgets(temp, sizeof(temp), fp)) {
        strcat(buffer, temp); //entire output
        token = strtok(temp, " ");
        if (token != NULL) {
            strcat(buffer, "Current time: ");
            strcat(buffer, token);  
            strcat(buffer, "\n");
        }
        token = strtok(NULL, " "); //ignore second token        
        
        token = strtok(NULL, ":");
        if (token != NULL) {
        strcat(buffer, "Uptime: ");   
            strcat(buffer, token);  
             strcat(buffer, "hours\b");
        }
        token = strtok(NULL, ",");
          if (token != NULL) {
            strcat(buffer, token);  
             strcat(buffer, "minutes");
             strcat(buffer, "\n");
        }
        token = strtok(NULL, ",");
        if (token != NULL) {
            strcat(buffer, "Number of Users: ");
            strcat(buffer, token);  
            strcat(buffer, "\n");
        }
        
        token = strtok(NULL, ":"); //ignore 
          token = strtok(NULL, "\0");
        if (token != NULL) {
        char *newline = strchr(token, '\n');
    if (newline) 
    	*newline = '\0'; // Replace \n with \0
            strcat(buffer, "Average load:\t");
            strcat(buffer, token);  
            strcat(buffer, " for 1min, 5min and 15min respectively\n");   
        } 
        }
            pclose(fp);
    }

    // Running processes
    strcat(buffer, "\nRunning Processes:\n");
    fp = popen("ps -e| wc -l", "r");
    if (fp) {
        while (fgets(temp, sizeof(temp), fp)) {
            int num = atoi(temp); //convert string to numeric
        num -= 1; // Subtract 1 for the header line
         char count[10];
        sprintf(count, "%d\n", num);
        strcat(buffer,count);
        }
        pclose(fp);
    }

// Network Activity
strcat(buffer, "\nNetwork Activity:\n");
fp = popen("netstat -i", "r");
if (fp) {
    int line_number = 0;
    while (fgets(temp, sizeof(temp), fp)) {
        if (line_number == 0) {
            // Skip header
            line_number++;
            continue;
        }
        char iface[20];
        int mtu;
        unsigned long rx_ok, rx_err, rx_drp, rx_ovr;
        unsigned long tx_ok, tx_err, tx_drp, tx_ovr;
        
        // Parse fields based on netstat -i output
        sscanf(temp, "%s %d %lu %lu %lu %lu %lu %lu %lu %lu", 
            iface, &mtu, &rx_ok, &rx_err, &rx_drp, &rx_ovr, &tx_ok, &tx_err, &tx_drp, &tx_ovr);

        strcat(buffer, "Interface: ");
        strcat(buffer, iface);
        strcat(buffer, "\n");

        char temp_buf[512];
        snprintf(temp_buf, sizeof(temp_buf),
                 "RX Packets: %lu, RX Errors: %lu\n"
                 "TX Packets: %lu, TX Errors: %lu\n\n",
                 rx_ok, rx_err, tx_ok, tx_err);
        strcat(buffer, temp_buf);

        line_number++;
    }
    pclose(fp);
}


    // System Event Logs (dmesg)
   strcat(buffer, "\nSystem Event Logs:\n");
    fp = popen(" dmesg | tail -n 5", "r");  // Fetch last 10 events
    if (fp) {
        while (fgets(temp, sizeof(temp), fp)) {
            strcat(buffer, temp);
        }
        pclose(fp);
    }
    
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

