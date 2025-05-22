#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 12345
#define CHUNK_SIZE 5
#define RETRANSMIT_INTERVAL_SEC 5
#define TIMEOUT_USEC 500000
#define MAX_CHUNKS 100

int sockfd;
struct sockaddr_in client_addr, client_addr;
socklen_t addr_len = sizeof(client_addr);

void server_send();
void server_recv();

struct packet
{
    int seq_num;
    int total_chunks;
    char data[CHUNK_SIZE];
};

// void set_socket_non_blocking(int sockfd)
// {
//     int flags = fcntl(sockfd, F_GETFL, 0);
//     if (flags == -1)
//     {
//         perror("fcntl get failed");
//         exit(EXIT_FAILURE);
//     }

//     flags |= O_NONBLOCK;
//     if (fcntl(sockfd, F_SETFL, flags) == -1)
//     {
//         perror("fcntl set failed");
//         exit(EXIT_FAILURE);
//     }
// }

void server_send()
{
    printf("\n..........................................\n\n");
    printf("Enter message to send to client:\n");
    char *message = (char *)malloc(4096 * sizeof(char));
    int x = 0;
    while (1)
    {
        char ch;
        scanf("%c", &ch);
        if (ch == '\n')
        {
            break;
        }
        message[x] = ch;
        x++;
    }

    struct timeval select_timeout = {0, TIMEOUT_USEC};
    struct packet send_packet;
    int total_chunks;
    int ack;
    int chunk_status[MAX_CHUNKS];
    struct timeval last_sent_time[MAX_CHUNKS];

    memset(chunk_status, 0, sizeof(chunk_status));
    memset(last_sent_time, 0, sizeof(last_sent_time));

    int message_len = strlen(message);
    total_chunks = (message_len + CHUNK_SIZE - 1) / CHUNK_SIZE;

    for (int seq_num = 0; seq_num < total_chunks; seq_num++)
    {
        send_packet.seq_num = seq_num;
        send_packet.total_chunks = total_chunks;
        memset(send_packet.data, 0, CHUNK_SIZE + 1);
        strncpy(send_packet.data, message + seq_num * CHUNK_SIZE, CHUNK_SIZE);
        sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr *)&client_addr, addr_len);
        chunk_status[seq_num] = 1;
        printf("Sent chunk %d\n", seq_num);
        printf("%s\n", send_packet.data);
        gettimeofday(&last_sent_time[seq_num], NULL);
    }

    while (1)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        int activity = select(sockfd + 1, &readfds, NULL, NULL, &select_timeout);
        if (activity < 0)
        {
            perror("Select error");
            break;
        }

        if (FD_ISSET(sockfd, &readfds))
        {
            if (recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&client_addr, &addr_len) > 0)
            {
                if (ack >= 0 && ack < total_chunks)
                {
                    chunk_status[ack] = 2;
                    printf("Received ACK for chunk %d\n", ack);
                }
            }
        }

        struct timeval current_time;
        gettimeofday(&current_time, NULL);

        for (int seq_num = 0; seq_num < total_chunks; seq_num++)
        {
            if (chunk_status[seq_num] == 1)
            {
                long elapsed_time_sec = (current_time.tv_sec - last_sent_time[seq_num].tv_sec);
                if (elapsed_time_sec >= RETRANSMIT_INTERVAL_SEC)
                {
                    printf("Timeout for chunk %d, resending...\n", seq_num);
                    send_packet.seq_num = seq_num;
                    send_packet.total_chunks = total_chunks;
                    memset(send_packet.data, 0, CHUNK_SIZE);
                    strncpy(send_packet.data, message + seq_num * CHUNK_SIZE, CHUNK_SIZE);
                    sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr *)&client_addr, addr_len);
                    gettimeofday(&last_sent_time[seq_num], NULL);
                }
            }
        }

        int check = 1;
        for (int seq_num = 0; seq_num < total_chunks; seq_num++)
        {
            if (chunk_status[seq_num] != 2)
            {
                check = 0;
                break;
            }
        }

        if (check)
        {
            printf("All chunks acknowledged.\n");
            server_recv();
        }
    }

    close(sockfd);
    return;
}

void server_recv()
{
    printf("\nWaiting for Client's Message:\n");
    struct packet recv_packet;
    char **chunks = NULL;
    int expected_chunks = 0;
    int received_chunks = 0;
    int k = 0;
    while (1)
    {
        k++;
        int recv_size = recvfrom(sockfd, &recv_packet, sizeof(recv_packet), 0, (struct sockaddr *)&client_addr, &addr_len);
        if (recv_size < 0)
        {
            continue;
        }

        if (expected_chunks == 0)
        {
            expected_chunks = recv_packet.total_chunks;
            chunks = (char **)malloc(expected_chunks * sizeof(char *));
            for (int i = 0; i < expected_chunks; i++)
            {
                chunks[i] = NULL;
            }
        }

        //   if(k == 2 || k == 6)
        // {
        //     continue;
        // }

        if (chunks[recv_packet.seq_num] == NULL)
        {
            chunks[recv_packet.seq_num] = strdup(recv_packet.data);
            received_chunks++;
            printf("%s\n", chunks[recv_packet.seq_num]);
            printf("Received chunk %d %d\n", recv_packet.seq_num, k);
        }

        sendto(sockfd, &recv_packet.seq_num, sizeof(recv_packet.seq_num), 0, (struct sockaddr *)&client_addr, addr_len);

        if (received_chunks == expected_chunks)
        {
            printf("\nAll chunks received.\n");
            printf("\nReassembled meassage:\n");
            for (int i = 0; i < expected_chunks; i++)
            {
                printf("%s", chunks[i]);
                free(chunks[i]);
            }
            printf("\n");

            free(chunks);
            server_send();
        }
    }

    close(sockfd);
    return;
}

int main()
{
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // set_socket_non_blocking(sockfd);
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    server_recv();
    return 0;
}
