#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

char board[3][3];
int player_num = 0;

void init()
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

void displayBoard(int client_fd1, int client_fd2)
{
    char current_board[4096];
    snprintf(current_board, 4096, "\n %c | %c | %c\n %c | %c | %c\n %c | %c | %c\n",
             board[0][0], board[0][1], board[0][2], board[1][0], board[1][1], board[1][2],
             board[2][0], board[2][1], board[2][2]);
    send(client_fd1, current_board, strlen(current_board), 0);
    send(client_fd2, current_board, strlen(current_board), 0);
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
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    goto func;
    
func:
    printf("Waiting for Player 1...\n");

    int new_socket1;
    if ((new_socket1 = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("Accept Player 1 failed");
        exit(EXIT_FAILURE);
    }

    printf("Player 1 Connected.\n");
    send(new_socket1, "Welcome Player 1! You are X\n", 29, 0);

    printf("Waiting for Player 2...\n");

    int new_socket2;
    if ((new_socket2 = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("Accept Player 2 failed");
        exit(EXIT_FAILURE);
    }

    printf("Player 2 Connected.\n");
    send(new_socket2, "Welcome Player 2! You are O\n", 29, 0);

    while (1)
    {
        init();
        player_num = 0;
        displayBoard(new_socket1, new_socket2);
        while (1)
        {
            while (1)
            {
                snprintf(buffer, 4096, "Player %d's Move.\n", player_num + 1);
                send(new_socket1, buffer, strlen(buffer), 0);
                snprintf(buffer, 4096, "Player %d's Move.\n", player_num + 1);
                send(new_socket2, buffer, strlen(buffer), 0);

                snprintf(buffer, 4096, "Player %d, enter your move (row and column): ", player_num + 1);
                if (player_num == 0)
                {
                    send(new_socket1, buffer, strlen(buffer), 0);
                }
                else
                {
                    send(new_socket2, buffer, strlen(buffer), 0);
                }

                if (player_num == 0)
                {
                    recv(new_socket1, buffer, 4096, 0);
                }
                else
                {
                    recv(new_socket2, buffer, 4096, 0);
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
                        send(new_socket1, buffer, strlen(buffer), 0);
                    }
                    else
                    {
                        send(new_socket2, buffer, strlen(buffer), 0);
                    }
                }
            }

            displayBoard(new_socket1, new_socket2);
            int result = check();

            if (result == 1 || result == 2)
            {
                snprintf(buffer, 4096, "Player %d Wins!\n", result);
                send(new_socket1, buffer, strlen(buffer), 0);
                send(new_socket2, buffer, strlen(buffer), 0);
                break;
            }
            else if (result == 0)
            {
                snprintf(buffer, 4096, "It's a Draw!\n");
                send(new_socket1, buffer, strlen(buffer), 0);
                send(new_socket2, buffer, strlen(buffer), 0);
                break;
            }
            else
            {
                if (player_num == 1)
                {
                    player_num = 0;
                }
                else
                {
                    player_num = 1;
                }
            }
        }

        snprintf(buffer, 4096, "Do you want to play again? (yes/no): ");

        send(new_socket1, buffer, strlen(buffer), 0);
        send(new_socket2, buffer, strlen(buffer), 0);

        char response1[4096], response2[4096];

        recv(new_socket1, response1, 4096, 0);
        recv(new_socket2, response2, 4096, 0);

        if (strncmp(response1, "yes", 3) == 0 && strncmp(response2, "yes", 3) == 0)
        {
            snprintf(buffer, 4096, "Starting New game!.\n");
            send(new_socket1, buffer, strlen(buffer), 0);
            send(new_socket2, buffer, strlen(buffer), 0);
            continue;
        }
        else
        {
            if (strncmp(response1, "yes", 3) == 0 && strncmp(response2, "no", 2) == 0)
            {
                snprintf(buffer, 4096, "Your opponent did not wish to play.\n");
                send(new_socket1, buffer, strlen(buffer), 0);
            }
            else if (strncmp(response1, "no", 2) == 0 && strncmp(response2, "yes", 3) == 0)
            {
                snprintf(buffer, 4096, "Your opponent did not wish to play.\n");
                send(new_socket2, buffer, strlen(buffer), 0);
            }

            snprintf(buffer, 4096, "Game over. Closing connection.\n");
            send(new_socket1, buffer, strlen(buffer), 0);
            send(new_socket2, buffer, strlen(buffer), 0);
            close(new_socket1);
            close(new_socket2);
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
