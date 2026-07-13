#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <IP address> <port>\n", argv[0]);
        exit(1);
    }

    char *ipAddress = argv[1];
    int port = atoi(argv[2]);

    int clientSocket;
    struct sockaddr_in serverAddr;
    char buf[1024];

    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    printf("Client Socket Created Successfully...\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ipAddress);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect failed");
        exit(1);
    }
    printf("Connected To Server Successfully...\n");

    memset(buf, '\0', sizeof(buf));
    recv(clientSocket, buf, 1024, 0);
    printf("Data Recieved: %s...\n", buf);
    printf("Closing Connection...\n");

    close(clientSocket);
    return 0;
}
