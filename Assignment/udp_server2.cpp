#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fstream>
#include <cassert>
#define DATA_LEN 500

struct ack_so   {
    uint8_t num;
    uint8_t len;
};

int main(void){
    // Create UDP socket:
    int socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    assert(socket_desc >= 0);
    printf("Socket created successfully\n");

    // Set port and IP and bind to it:
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1053);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    assert(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) >= 0);
    printf("Done with binding\nListening for incoming messages...\n\n");

    std::ofstream MyFile("myTCPreceive.txt");
    int number_of_packets_received = 0;
    int upper_bound = 1;
    int n = 0;

    while (1) {
        struct sockaddr_in client_addr;
        char client_message[DATA_LEN];
        int client_struct_length = sizeof(client_addr);
        
        memset(client_message, '\0', sizeof(client_message));
        assert((n += recvfrom(socket_desc, client_message, sizeof(client_message), 0, (struct sockaddr*)&client_addr, (socklen_t *) &client_struct_length)) >= 0);
        number_of_packets_received += 1;
        MyFile << client_message;

        if (number_of_packets_received < upper_bound) continue; // Send an ACK only when we hit the upper bound.
        
        number_of_packets_received = 0;
        upper_bound = upper_bound == 3 ? 1 : upper_bound + 1;
        
        struct ack_so acknowledgement;
        acknowledgement.len = 1;
        acknowledgement.num = 2;
        assert(sendto(socket_desc, &acknowledgement, sizeof(acknowledgement), 0, (struct sockaddr*)&client_addr, client_struct_length) >= 0);
    }
    return 0;
}