#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define HOSTNAME "ua.pool.ntp.org"
#define PORT 123

#define NTP_TIMESTAMP_DELTA 2208988800ull

typedef struct
{

	uint8_t li_vn_mode; // Eight bits. li, vn, and mode.
						// li.   Two bits.   Leap indicator.
						// vn.   Three bits. Version number of the protocol.
						// mode. Three bits. Client will pick mode 3 for client.

	uint8_t stratum;   // Eight bits. Stratum level of the local clock.
	uint8_t poll;	   // Eight bits. Maximum interval between successive messages.
	uint8_t precision; // Eight bits. Precision of the local clock.

	uint32_t rootDelay;		 // 32 bits. Total round trip delay time.
	uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
	uint32_t refId;			 // 32 bits. Reference clock identifier.

	uint32_t refTm_s; // 32 bits. Reference time-stamp seconds.
	uint32_t refTm_f; // 32 bits. Reference time-stamp fraction of a second.

	uint32_t origTm_s; // 32 bits. Originate time-stamp seconds.
	uint32_t origTm_f; // 32 bits. Originate time-stamp fraction of a second.

	uint32_t rxTm_s; // 32 bits. Received time-stamp seconds.
	uint32_t rxTm_f; // 32 bits. Received time-stamp fraction of a second.

	uint32_t txTm_s; // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
	uint32_t txTm_f; // 32 bits. Transmit time-stamp fraction of a second.

} ntp_packet; // Total: 384 bits or 48 bytes.

void get_ip_address_by_hostname(char *hostname, char ip[]);

int main(int argc, char *argv[]) {
	int n;
	char command[100];
	char IP[100];
	get_ip_address_by_hostname(HOSTNAME, IP);

	ntp_packet packet = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	memset(&packet, 0, sizeof(ntp_packet));

	*((char *)&packet + 0) = 0x1b;
	int socket_desc;
	struct sockaddr_in server;

	socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // AF_INET (this is IP version 4)

	if (socket_desc == -1) {
		printf("Could not create socket");
		return 1;
	}

	server.sin_addr.s_addr = inet_addr(IP);
	server.sin_family = AF_INET;
	server.sin_port = htons(123);

	if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
		printf("connect error");
		return 1;
	}

	n = write(socket_desc, (char *)&packet, sizeof(ntp_packet));
	if (n < 0) {
		printf("ERROR writing to socket");
		return 1;
	}
	n = read(socket_desc, (char *)&packet, sizeof(ntp_packet));
	if (n < 0) {
		printf("ERROR writing to socket");
		return 1;
	}

	packet.txTm_s = ntohl(packet.txTm_s);
	packet.txTm_f = ntohl(packet.txTm_f);

	time_t txTm = (time_t)(packet.txTm_s - NTP_TIMESTAMP_DELTA);
	sprintf(command, "sudo date -s '%s'", ctime((const time_t *)&txTm));
	system(command);
	printf("Time: %s", ctime((const time_t *)&txTm));
	return 0;
}



void get_ip_address_by_hostname(char *hostname, char ip[]) {
	struct hostent *he;
	struct in_addr **addr_list;
	int i;

	if ((he = gethostbyname(hostname)) == NULL) {
		printf("Error while defining the IP address");
		return;
	}

	addr_list = (struct in_addr **)he->h_addr_list;

	for (i = 0; addr_list[i] != NULL; i++) {
		strcpy(ip, inet_ntoa(*addr_list[i]));
	}
	printf("Hostname: %s - IP : %s\n", hostname, ip);
}