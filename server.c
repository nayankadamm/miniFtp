#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
 
#define PORT 8080
#define BUF_SIZE 1024
#define END_OF_TRANSFER "END_OF_TRANSFER"
 
void handle_upload(int new_socket);
void handle_download(int new_socket);
void handle_list(int new_socket);
void handle_delete(int new_socket);
 
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUF_SIZE] = {0};
 
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
 
    // Bind socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
 
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
 
    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
 
    printf("Server is listening on port %d\n", PORT);
 
    while (1) {
        // Accept incoming connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
 
        // Receive command
        memset(buffer, 0, BUF_SIZE);
        read(new_socket, buffer, BUF_SIZE);
        printf("Received command: %s\n", buffer);
 
        if (strncmp(buffer, "UPLOAD", 6) == 0) {
            send(new_socket, "ACK", 3, 0);  // Send acknowledgment
            handle_upload(new_socket);
        } else if (strncmp(buffer, "DOWNLOAD", 8) == 0) {
            send(new_socket, "ACK", 3, 0);  // Send acknowledgment
            handle_download(new_socket);
        } else if (strncmp(buffer, "LIST", 4) == 0) {
            handle_list(new_socket);
        } else if (strncmp(buffer, "DELETE", 6) == 0) {
            send(new_socket, "ACK", 3, 0);  // Send acknowledgment
            handle_delete(new_socket);
        } else {
            printf("Invalid command received.\n");
        }
 
        close(new_socket);
    }
 
    close(server_fd);
    return 0;
}
 
void handle_upload(int new_socket) {
    char buffer[BUF_SIZE] = {0};
    char filename[BUF_SIZE] = {0};
    int bytes_read, file_fd;
 
    // Receive filename
    read(new_socket, filename, BUF_SIZE);
    file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
 
    if (file_fd < 0) {
        perror("File open failed");
        return;
    }
 
    // Receive file data
    while ((bytes_read = read(new_socket, buffer, BUF_SIZE)) > 0) {
        if (strncmp(buffer, END_OF_TRANSFER, strlen(END_OF_TRANSFER)) == 0) {
            break;
        }
        write(file_fd, buffer, bytes_read);
    }
 
    close(file_fd);
    printf("File %s uploaded successfully.\n", filename);
}
 
void handle_download(int new_socket) {
    char buffer[BUF_SIZE] = {0};
    char filename[BUF_SIZE] = {0};
    int bytes_read, file_fd;
 
    // Receive filename
    read(new_socket, filename, BUF_SIZE);
    file_fd = open(filename, O_RDONLY);
 
    if (file_fd < 0) {
        perror("File open failed");
        return;
    }
 
    // Send file data
    while ((bytes_read = read(file_fd, buffer, BUF_SIZE)) > 0) {
        send(new_socket, buffer, bytes_read, 0);
    }
 
    // Send end of transfer signal
    send(new_socket, END_OF_TRANSFER, strlen(END_OF_TRANSFER), 0);
 
    close(file_fd);
    printf("File %s downloaded successfully.\n", filename);
}
 
void handle_list(int new_socket) {
    struct dirent *de;
    DIR *dr = opendir(".");
    char buffer[BUF_SIZE] = {0};
 
    if (dr == NULL) {
        perror("Could not open directory");
        return;
    }
 
    while ((de = readdir(dr)) != NULL) {
        strcat(buffer, de->d_name);
        strcat(buffer, "\n");
    }
 
    write(new_socket, buffer, strlen(buffer));
    closedir(dr);
    printf("Directory listing sent.\n");
}
 
void handle_delete(int new_socket) {
    char filename[BUF_SIZE] = {0};
 
    // Receive filename
    read(new_socket, filename, BUF_SIZE);
 
    if (remove(filename) == 0) {
        printf("File %s deleted successfully.\n", filename);
    } else {
        perror("File delete failed");
    }
}
 