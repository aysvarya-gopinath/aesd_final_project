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
#define MAX_METRICS 1024
#define BACKLOG 10

void get_system_metrics(int client_fd)
{
    FILE *fp;
    char temp[256];
    char *token;
    char buffer[MAX_METRICS];

    buffer[0] = '\0'; // clear buffer

    // CPU usage
    strcat(buffer, "CPU Usage:\n");
   
    fp = fopen("/proc/stat", "r"); // open /proc/stat
    if (fp)
    {
        if (fgets(temp, sizeof(temp), fp)) //read a line from the file
        {
            if (strncmp(temp, "cpu ", 4) == 0) // only match "cpu " line
            {
                strcat(buffer, temp); 

                unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
                 //read the formatted input and store in appropriate variables
                sscanf(temp, "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                       &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice); 

                // write each field separately in this format to the buffer
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

                // CPU usage
                unsigned long long idle_time = idle + iowait; //idle time
                unsigned long long non_idle = user + nice + system + irq + softirq + steal; //running
                unsigned long long total = idle_time + non_idle;  //total time

                float cpu_usage = (float)(total - idle_time) / total * 100.0;  //usage percentage
                snprintf(cpu_info, sizeof(cpu_info), "CPU Usage percent: %.2f%%\n", cpu_usage);
                strcat(buffer, cpu_info);
            }
        }
        fclose(fp);
        send(client_fd, buffer, strlen(buffer), 0);
    }

    // Memory usage
    buffer[0] = '\0';  //clear buffer
    strcat(buffer, "\nMemory Usage:\n");
    fp = popen("free -h", "r");
    if (fp)
    {
        while (fgets(temp, sizeof(temp), fp)!= NULL)
        {
            strcat(buffer, temp);
        }
        pclose(fp);
        send(client_fd, buffer, strlen(buffer), 0);
    }

    // Disk usage
    buffer[0] = '\0';  //clear buffer
    strcat(buffer, "\nDisk Usage:\n");
    fp = popen("df -h", "r");
    if (fp)
    {
        while (fgets(temp, sizeof(temp), fp)!= NULL)
        {
            strcat(buffer, temp);
        }
        pclose(fp);
        send(client_fd, buffer, strlen(buffer), 0);
    }

    // Uptime
    buffer[0] = '\0';  //clear buffer
    strcat(buffer, "\nSystem timings:\n");
    fp = popen("uptime", "r"); // uptime -p
    if (fp)
    {
        while (fgets(temp, sizeof(temp), fp)!= NULL)
        {
            strcat(buffer, temp); // entire output
            token = strtok(temp, " ");
            if (token != NULL)
            {
                strcat(buffer, "Current time: ");
                strcat(buffer, token);
                strcat(buffer, "\n");
            }
            token = strtok(NULL, " "); // ignore second token

            token = strtok(NULL, ":");
            if (token != NULL)
            {
                strcat(buffer, "Uptime: ");
                strcat(buffer, token);
                strcat(buffer, "hours\b");
            }
            token = strtok(NULL, ",");
            if (token != NULL)
            {
                strcat(buffer, token);
                strcat(buffer, "minutes");
                strcat(buffer, "\n");
            }
            token = strtok(NULL, ",");
            if (token != NULL)
            {
                strcat(buffer, "Number of Users: ");
                strcat(buffer, token);
                strcat(buffer, "\n");
            }

            token = strtok(NULL, ":"); // ignore
            token = strtok(NULL, "\0");
            if (token != NULL)
            {
                char *newline = strchr(token, '\n');
                if (newline)
                    *newline = '\0'; // Replace \n with \0
                strcat(buffer, "Average load:\t");
                strcat(buffer, token);
                strcat(buffer, " for 1min, 5min and 15min respectively\n");
            }
        }
        pclose(fp);
        send(client_fd, buffer, strlen(buffer), 0);
    }

    // Running processes
    buffer[0] = '\0';  //clear buffer
    strcat(buffer, "\nRunning Processes:\n");
    fp = popen("ps -e| wc -l", "r");
    if (fp)
    {
        while (fgets(temp, sizeof(temp), fp)!= NULL)
        {
            int num = atoi(temp); // convert string to numeric
            num -= 1;             // Subtract 1 for the header line
            char count[10];
            sprintf(count, "%d\n", num);
            strcat(buffer, count);
        }
        pclose(fp);
        send(client_fd, buffer, strlen(buffer), 0);
    }

    // Network Activity
    buffer[0] = '\0';  //clear buffer
    strcat(buffer, "\nNetwork Activity:\n");
    fp = popen("netstat -i", "r");
    if (fp)
    {
        int line_number = 0;
        while (fgets(temp, sizeof(temp), fp)!= NULL)
        {
            if (line_number == 0)
            {
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
        send(client_fd, buffer, strlen(buffer), 0);
    }

    // System Event Logs (dmesg)
    buffer[0] = '\0';  //clear buffer
    strcat(buffer, "\nSystem Event Logs:\n");
    fp = popen(" sudo dmesg | tail -n 5", "r"); // Fetch last 10 events
    if (fp)
    {
        while (fgets(temp, sizeof(temp), fp)!= NULL)
        {
            strcat(buffer, temp);
        }
        pclose(fp);
        send(client_fd, buffer, strlen(buffer), 0);
    }
}

int main()
{
    int server_fd, new_fd, yes = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
   

    // create socket for the server
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0)
    {
        perror("Socket failed");
        return -1;
    }
    
//resuse the port
    if (setsockopt(server_fd, SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT, &yes,sizeof(yes)))
    {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

    // sockaddr data structure holds socket address information for IPV4
    address.sin_family = AF_INET; //communicating between processes on different hosts connected by IPV4
    address.sin_addr.s_addr = INADDR_ANY; //bind the server to the localhost
    address.sin_port = htons(PORT); // host byte order to network byte order
    
    // bind socket to IP address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        return -1;
    }

    // server listen for connection
    if (listen(server_fd, BACKLOG) < 0)
    {
        perror("Listen failed"); // waits for client to approach
        close(server_fd);
        return -1;
    }

    printf("Server listening on port %d...\n", PORT);

    // accept client connection
    new_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (new_fd < 0)
    {
        perror("Accept failed");
        return -1;
    }

    // calculate the system metrics
    get_system_metrics(new_fd);
    printf("System metrics sent to client.\n");

    close(new_fd);
    close(server_fd);
    return 0;
}

