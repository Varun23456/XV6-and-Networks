#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


void client()
{
    char buffer[4096];
    int sock;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("Socket creation error\n");
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sendto(sock, "Hello, server!", 14, 0, (struct sockaddr *)&addr, addr_len);

    while (1)
    {
        int size = recvfrom(sock, buffer, 4096, 0, (struct sockaddr *)&addr, &addr_len);
        buffer[size] = '\0';
        printf("%s", buffer);

        if (strncmp(buffer, "Game over", 9) == 0)
        {
            break;
        }

        if (strstr(buffer, "enter your move") != NULL || strstr(buffer, "Do you want to play again?") != NULL)
        {
            fgets(buffer, 4096, stdin);
            sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&addr, addr_len);
        }
    }

    close(sock);
    return;
}

int main()
{
    client();
    return 0;
}
