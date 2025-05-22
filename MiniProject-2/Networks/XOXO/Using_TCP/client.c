#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void client()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[4096] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    while (1)
    {
        int size = read(sock, buffer, 4096);
        buffer[size] = '\0';
        printf("%s", buffer);

        if (strncmp(buffer, "Your opponent did not wish to play.", 35) == 0 || strncmp(buffer, "Game over",9) == 0)
        {
            break;
        }

        if (strstr(buffer, "enter your move") != NULL || strstr(buffer,"Do you want to play again?") != NULL)
        {
            fgets(buffer, 4096, stdin);
            send(sock, buffer, strlen(buffer), 0);
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
