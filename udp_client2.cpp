#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <cassert>
#include <string>

struct ack_so   {
    uint8_t num;
    uint8_t len;
};

void tv_sub(struct  timeval *out, struct timeval *in)   {
	if ((out->tv_usec -= in->tv_usec) < 0)   {
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

float sendFile(int socket_desc, struct sockaddr_in server_addr, int server_struct_length, long* length, int data_len) {
    // Process the file.
    FILE *fp = fopen("myfile.txt", "r+t");
    assert(fp != nullptr);
    fseek(fp, 0, 2);
    long file_size = ftell(fp);
    *length = file_size;
    printf("The file size is %ld bytes \n", file_size);
    rewind(fp);

    // Copy the file data into a char array.
    char client_message[file_size];
    memset(client_message, '\0', sizeof(client_message));
    fread(client_message, 1, file_size, fp);

    struct timeval sendt, recvt;

    // [ci] represents the number of bytes that has been sent.
    long ci = 0;
    gettimeofday(&sendt, nullptr);

    int number_of_packets_sent = 0;
    int upper_bound = 1;
    while (ci <= file_size) {
        // Determine the length of the file to send and copy it onto the buffer. We set the limit to 500 bytes.
        char sends[data_len];
		long slen = ((file_size + 1 - ci) <= data_len) ? file_size + 1 - ci : data_len;
		memcpy(sends, client_message + ci, slen);

        // Send the file to the server.
        assert((ci += sendto(socket_desc, sends, slen, 0, (struct sockaddr*)&server_addr, server_struct_length)) >= 0);

        number_of_packets_sent += 1;
        if (number_of_packets_sent < upper_bound) continue;

        number_of_packets_sent = 0;
        upper_bound = upper_bound == 3 ? 1 : upper_bound + 1;

        // [acknowledgement] represents the ACK packet that we will get from the server.
        struct ack_so acknowledgement;
        assert(recvfrom(socket_desc, &acknowledgement, sizeof(acknowledgement), 0, (struct sockaddr*)&server_addr, (socklen_t *) &server_struct_length) >= 0);
    }
    printf("The value of ci is %ld\n", ci);
    gettimeofday(&recvt, nullptr);
    tv_sub(&recvt, &sendt);
    return (recvt.tv_sec) * 1000.0 + (recvt.tv_usec) / 1000.0;
}

int main(int argc, char* argv[])  { 
    assert(argc >= 2);
    // Create socket:
    int socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    assert(socket_desc >= 0);
    printf("Socket created successfully\n");
    
    // Set port and IP:
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1053);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int server_struct_length = sizeof(server_addr);

    long length;
    float time = sendFile(socket_desc, server_addr, server_struct_length, &length, std::stoi(argv[1]));
    printf("Transmission Time (ms):%.3f\n", time);
    printf("Data Length (bytes): %ld\n", length);
    printf("Data Rate (kbps): %.3f", length / (float) time);
    return 0;
}
