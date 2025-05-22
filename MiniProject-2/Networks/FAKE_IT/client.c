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
#define MAX_CHUNKS 1000

int sockfd;
struct sockaddr_in server_addr;
socklen_t addr_len = sizeof(server_addr);

void client_send();
void client_recv();

struct packet
{
    int seq_num;
    int total_chunks;
    char data[CHUNK_SIZE + 1];
};


void client_recv()
{
    printf("\nWaiting for Server's Message:\n");
    struct packet recv_packet;
    char **chunks = NULL;
    int expected_chunks = 0;
    int received_chunks = 0;

    int k = 0;
    while (1)
    {
        k++;
        int recv_size = recvfrom(sockfd, &recv_packet, sizeof(recv_packet), 0, (struct sockaddr *)&server_addr, &addr_len);
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

        // if(k == 2 || k == 6)
        // {
        //     continue;
        // }


        if (chunks[recv_packet.seq_num] == NULL)
        {
            chunks[recv_packet.seq_num] = strdup(recv_packet.data);
            received_chunks++;
            printf("%s\n", chunks[recv_packet.seq_num]);
            printf("Received chunk %d\n", recv_packet.seq_num);
        }

        sendto(sockfd, &recv_packet.seq_num, sizeof(recv_packet.seq_num), 0, (struct sockaddr *)&server_addr, addr_len);

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
            client_send();
        }
    }

    close(sockfd);
    return;
}

void client_send()
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
        sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr *)&server_addr, addr_len);
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
            if (recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&server_addr, &addr_len) > 0)
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
                    sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr *)&server_addr, addr_len);
                    gettimeofday(&last_sent_time[seq_num], NULL);
                }
            }
        }

        int all_ack_received = 1;
        for (int seq_num = 0; seq_num < total_chunks; seq_num++)
        {
            if (chunk_status[seq_num] != 2)
            {
                all_ack_received = 0;
                break;
            }
        }

        if (all_ack_received)
        {
            printf("All chunks acknowledged.\n");
            client_recv();
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
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    client_send();
    return 0;
}
