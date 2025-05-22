#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

char board[3][3];
int player_num = 0;

void initBoard()
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            board[i][j] = '-';
        }
    }

    return;
}

void displayBoard(int server_fd, struct sockaddr_in client_addr1, struct sockaddr_in client_addr2)
{
    char boardState[4096];
    snprintf(boardState, 4096, "\n %c | %c | %c\n %c | %c | %c\n %c | %c | %c\n",
             board[0][0], board[0][1], board[0][2], board[1][0], board[1][1], board[1][2],
             board[2][0], board[2][1], board[2][2]);

    sendto(server_fd, boardState, strlen(boardState), 0, (struct sockaddr *)&client_addr1, sizeof(client_addr1));
    sendto(server_fd, boardState, strlen(boardState), 0, (struct sockaddr *)&client_addr2, sizeof(client_addr2));
    return;
}

int check()
{
    int p = 0;
    int q = 0;
    if ((board[0][0] == 'X' && board[1][0] == 'X' && board[2][0] == 'X') || (board[0][0] == 'X' && board[0][1] == 'X' && board[0][2] == 'X'))
    {
        p = 1;
    }
    else if ((board[0][1] == 'X' && board[1][1] == 'X' && board[2][1] == 'X') || (board[1][0] == 'X' && board[1][1] == 'X' && board[1][2] == 'X'))
    {
        p = 1;
    }
    else if ((board[0][2] == 'X' && board[1][2] == 'X' && board[2][2] == 'X') || (board[2][0] == 'X' && board[2][1] == 'X' && board[2][2] == 'X'))
    {
        p = 1;
    }
    else if ((board[0][0] == 'X' && board[1][1] == 'X' && board[2][2] == 'X') || (board[0][2] == 'X' && board[1][1] == 'X' && board[2][0] == 'X'))
    {
        p = 1;
    }

    if ((board[0][0] == 'O' && board[1][0] == 'O' && board[2][0] == 'O') || (board[0][0] == 'O' && board[0][1] == 'O' && board[0][2] == 'O'))
    {
        q = 1;
    }
    else if ((board[0][1] == 'O' && board[1][1] == 'O' && board[2][1] == 'O') || (board[1][0] == 'O' && board[1][1] == 'O' && board[1][2] == 'O'))
    {
        q = 1;
    }
    else if ((board[0][2] == 'O' && board[1][2] == 'O' && board[2][2] == 'O') || (board[2][0] == 'O' && board[2][1] == 'O' && board[2][2] == 'O'))
    {
        q = 1;
    }
    else if ((board[0][0] == 'O' && board[1][1] == 'O' && board[2][2] == 'O') || (board[0][2] == 'O' && board[1][1] == 'O' && board[2][0] == 'O'))
    {
        q = 1;
    }

    if (p == 1 && q == 0)
    {
        return 1;
    }
    else if (p == 0 && q == 1)
    {
        return 2;
    }
    else if (p == 0 && q == 0)
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                if (board[i][j] == '-')
                {
                    return -1;
                }
            }
        }
        return 0;
    }
}

void server()
{
    char buffer[4096];
    int server_fd;

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(server_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    goto func;

func:    
    printf("Waiting for Player 1...\n");

    struct sockaddr_in client_addr1;
    socklen_t addr_len1 = sizeof(client_addr1);
    recvfrom(server_fd, buffer, 4096, 0, (struct sockaddr *)&client_addr1, &addr_len1);

    printf("Player 1 Connected.\n");
    sendto(server_fd, "Welcome Player 1! You are X\n", 29, 0, (struct sockaddr *)&client_addr1, addr_len1);

    printf("Waiting for Player 2...\n");

    struct sockaddr_in client_addr2;
    socklen_t addr_len2 = sizeof(client_addr2);
    recvfrom(server_fd, buffer, 4096, 0, (struct sockaddr *)&client_addr2, &addr_len2);

    printf("Player2 Connected.\n");
    sendto(server_fd, "Welcome Player 2! You are O\n", 29, 0, (struct sockaddr *)&client_addr2, addr_len2);

    while (1)
    {
        initBoard();
        player_num = 0;
        displayBoard(server_fd, client_addr1, client_addr2);

        while (1)
        {
            while (1)
            {
                snprintf(buffer, 4096, "Player %d, enter your move (row and column): ", player_num + 1);
                if (player_num == 0)
                {
                    sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr1, addr_len1);
                }
                else
                {
                    sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr2, addr_len2);
                }

                if (player_num == 0)
                {
                    recvfrom(server_fd, buffer, 4096, 0, (struct sockaddr *)&client_addr1, &addr_len1);
                }
                else
                {
                    recvfrom(server_fd, buffer, 4096, 0, (struct sockaddr *)&client_addr2, &addr_len2);
                }

                int row, col;
                sscanf(buffer, "%d %d", &row, &col);
                row = row -1;
                col = col -1;
                if (row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == '-')
                {
                    if (player_num == 0)
                    {
                        board[row][col] = 'X';
                    }
                    else
                    {
                        board[row][col] = 'O';
                    }
                    break;
                }
                else
                {
                    snprintf(buffer, 4096, "Invalid move. Try again.\n");
                    if (player_num == 0)
                    {
                        sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr1, addr_len1);
                    }
                    else
                    {
                        sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr2, addr_len2);
                    }
                }
            }

            displayBoard(server_fd, client_addr1, client_addr2);
            int result = check();

            if (result == 1 || result == 2)
            {
                snprintf(buffer, 4096, "Player %d Wins!\n", result);
                sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr1, addr_len1);
                sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr2, addr_len2);
                break;
            }
            else if (result == 0)
            {
                snprintf(buffer, 4096, "It's a Draw!\n");
                sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr1, addr_len1);
                sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr2, addr_len2);
                break;
            }
            else
            {
                if (player_num == 0)
                {
                    player_num = 1;
                }
                else
                {
                    player_num = 0;
                }
            }
        }

        snprintf(buffer, 4096, "Do you want to play again? (yes/no): ");

        sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr1, addr_len1);
        sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr2, addr_len2);

        char response1[4096], response2[4096];

        recvfrom(server_fd, response1, 4096, 0, (struct sockaddr *)&client_addr1, &addr_len1);
        recvfrom(server_fd, response2, 4096, 0, (struct sockaddr *)&client_addr2, &addr_len2);

        if (strncmp(response1, "yes", 3) == 0 && strncmp(response2, "yes", 3) == 0)
        {
            snprintf(buffer, 4096, "Starting New game!.\n");
            sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr1, addr_len1);
            sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr2, addr_len2);
            continue;
        }
        else
        {
            if (strncmp(response1, "yes", 3) == 0 && strncmp(response2, "no", 2) == 0)
            {
                snprintf(buffer, 4096, "Your opponent did not wish to play.\n");
                sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr1, addr_len1);
            }
            else if (strncmp(response1, "no", 2) == 0 && strncmp(response2, "yes", 3) == 0)
            {
                snprintf(buffer, 4096, "Your opponent did not wish to play.\n");
                sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr2, addr_len2);
            }

            snprintf(buffer, 4096, "Game over. Closing connection.\n");
            sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr1, addr_len1);
            sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr2, addr_len2);
            printf("Connection Closed.\n");
            printf("\n");
            goto func;
        }
    }

    return;
}

int main()
{
    server();
    return 0;
}
