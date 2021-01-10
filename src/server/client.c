#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr;
    // printf("%d" ,argv[1]);
    int port_val = 1500;
    char *ip_val = "127.0.0.1";
    int my_socket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_val);
    inet_pton(AF_INET, ip_val, &(server_addr.sin_addr));

    if (0 == connect(my_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)))
    {
    }
    else
    {
        printf("The problem number is %d", errno);
    }
    char buffer[50];
    int len = sprintf(buffer, "%s:%s", argv[1], argv[2]);
    buffer[len] = '\n';
    printf("%s",buffer);
    if (-1 == write(my_socket, buffer, 50))
    {
        perror("Them problem number is");
    }
    char in_buffer[100];
    char out_buffer[100];
    //read
    if (-1 == read(my_socket, in_buffer, 50))
    {
        printf("Them problem number is %d", errno);
    }
    printf("%s", in_buffer);
    close(my_socket);
}