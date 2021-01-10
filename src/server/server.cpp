#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <dirent.h>
#include <sys/types.h>
#include <iostream>
#include <sstream>
using namespace std;
string list_dir(const char *path)
{
    struct dirent *entry;
    DIR *dir = opendir(path);
    std::stringstream buffer;

    if (dir == NULL)
    {
        return "";
    }
    string result;
    result = result.c_str();
    while ((entry = readdir(dir)) != NULL)
    {
        buffer << (string)(entry->d_name) << endl;
    }
    closedir(dir);
    return buffer.str();
}
int main(int argc, char *argv[])
{

    char command[100];
    char nameOfDirFile[100];
    int num_1, num_2;
    struct sockaddr_in server_addr;
    int port_val = 1500;
    int my_socket = socket(AF_INET, SOCK_STREAM, 0);
    int nFoo = 1;
    setsockopt(my_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&nFoo,
               sizeof(nFoo));
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_val);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //bind
    if (-1 == bind(my_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)))
    {
        printf("Eror has occuerd: Bind not working :(\n");
    }
    //listen
    listen(my_socket, 200);
    socklen_t sock_size;
    sock_size = sizeof(struct sockaddr);
    while (1)
    {
        //accept
        int con_socket = accept(my_socket, (struct sockaddr *)&server_addr, &sock_size);
        char in_buffer[100];
        char out_buffer[100];
        //read
        if (-1 == read(con_socket, in_buffer, 10))
        {
            printf("Them problem number is %d", errno);
        }
        // printf("%s",in_buffer);
        char first_part[50];
        char second_part[50];
        sscanf(in_buffer, "%[^:]:%s", first_part, second_part);

        if (strcmp(first_part, "dir") == 0)
        {
            string result = list_dir("/home/kraven/Documents/SK2/Zaj2");
            cout << result;
            int sum = 32;
            int len = sprintf(out_buffer, "%s", result.c_str()
            );
            out_buffer[len] = '\n';
            if (-1 == write(con_socket, out_buffer, 10))
            {
                printf("Them problem number is %d", errno);
            }
        }

        // sscanf(command,first_part,second_part);
        // int sum = num_1 + num_2;
        // int len = sprintf(out_buffer, "%d", sum);
        // out_buffer[len] = '\n';
        // if (-1 == write(con_socket, out_buffer, 10))
        // {
        // printf("Them problem number is %d", errno);
        // }

        // printf("%d - %d\n", num_1, num_2);
        // printf("2)%s\n",command);
        printf("%s - %s\n", first_part, second_part);
    }

    close(my_socket);
}