#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
 
#define PORT 8080
#define BUF_SIZE 1024
#define END_OF_TRANSFER "END_OF_TRANSFER"
 
void upload_file(int sock);
void download_file(int sock);
void list_files(int sock);
void delete_file(int sock);
 
int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char command[BUF_SIZE] = {0};
 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
 
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
 
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }
 
    while (1) {
        printf("Enter command (UPLOAD/DOWNLOAD/LIST/DELETE/QUIT): ");
        fgets(command, BUF_SIZE, stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline character
 
        send(sock, command, strlen(command), 0);
 
        if (strncmp(command, "UPLOAD", 6) == 0) {
            upload_file(sock);
        } else if (strncmp(command, "DOWNLOAD", 8) == 0) {
            download_file(sock);
        } else if (strncmp(command, "LIST", 4) == 0) {
            list_files(sock);
        } else if (strncmp(command, "DELETE", 6) == 0) {
            delete_file(sock);
        } else if (strncmp(command, "QUIT", 4) == 0) {
            break;
        } else {
            printf("Invalid command.\n");
        }
 
        memset(command, 0, BUF_SIZE); // Clear command buffer
    }
 
    close(sock);
    return 0;
}
 
void upload_file(int sock) {
    char buffer[BUF_SIZE] = {0};
    char filename[BUF_SIZE] = {0};
    int file_fd, bytes_read;
 
    printf("Enter filename to upload: ");
    fgets(filename, BUF_SIZE, stdin);
    filename[strcspn(filename, "\n")] = 0;
 
    file_fd = open(filename, O_RDONLY |);
    if (file_fd < 0) {
        perror("File open failed");
        return;
    }
 
    send(sock, filename, strlen(filename), 0);
    read(sock, buffer, BUF_SIZE); // Wait for server acknowledgment
 
    while ((bytes_read = read(file_fd, buffer, BUF_SIZE)) > 0) {
        send(sock, buffer, bytes_read, 0);
    }
 
    // Indicate end of file transfer
    send(sock, END_OF_TRANSFER, strlen(END_OF_TRANSFER), 0);
 
    close(file_fd);
    printf("File %s uploaded successfully.\n", filename);
}
 
void download_file(int sock) {
    char buffer[BUF_SIZE] = {0};
    char filename[BUF_SIZE] = {0};
    int file_fd, bytes_read;
 
    printf("Enter filename to download: ");
    fgets(filename, BUF_SIZE, stdin);
    filename[strcspn(filename, "\n")] = 0;
 
    send(sock, filename, strlen(filename), 0);
    read(sock, buffer, BUF_SIZE); // Wait for server acknowledgment
 
    file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (file_fd < 0) {
        perror("File open failed");
        return;
    }
 
    while ((bytes_read = read(sock, buffer, BUF_SIZE)) > 0) {
        if (strncmp(buffer, END_OF_TRANSFER, strlen(END_OF_TRANSFER)) == 0) {
            break;
        }
        write(file_fd, buffer, bytes_read);
    }
 
    close(file_fd);
    printf("File %s downloaded successfully.\n", filename);
}
 
void list_files(int sock) {
    char buffer[BUF_SIZE] = {0};
    int bytes_read;
 
    while ((bytes_read = read(sock, buffer, BUF_SIZE)) > 0) {
        printf("%s", buffer);
        if (bytes_read < BUF_SIZE) {
            break;
        }
    }
}
 
void delete_file(int sock) {
    char filename[BUF_SIZE] = {0};
    char buffer[BUF_SIZE] = {0};
 
    printf("Enter filename to delete: ");
    fgets(filename, BUF_SIZE, stdin);
    filename[strcspn(filename, "\n")] = 0;
 
    send(sock, filename, strlen(filename), 0);
    read(sock, buffer, BUF_SIZE); // Wait for server acknowledgment
    printf("Delete request for file %s sent.\n", filename);
}