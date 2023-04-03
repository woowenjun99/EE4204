#include "headsock.h"

void str_cli1(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, int *len);                

int main(int argc, char *argv[])    {
	int len;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in ser_addr;
	struct hostent *sh = gethostbyname(argv[1]);
	struct in_addr **addrs = (struct in_addr **)sh->h_addr_list;

	if (argc!= 2)   {
		printf("parameters not match.");
		exit(0);
	}

	if (sh == nullptr) {             //get host's information
		printf("error when gethostbyname");
		exit(0);
	}

	if (sockfd<0)   {
		printf("error in socket");
		exit(1);
	}

	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);

	str_cli1(stdin, sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in), &len);   // receive and send

	close(sockfd);
	exit(0);
}

void str_cli1(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, int *len)   {
	char sends[MAXSIZE];

	printf("Please input a string (less than 50 characters):\n");
	if (fgets(sends, MAXSIZE, fp) == NULL) {
		printf("error input\n");
	}

	sendto(sockfd, &sends, strlen(sends), 0, addr, addrlen);                         //send the packet to server
	printf("send out!!\n");
}
