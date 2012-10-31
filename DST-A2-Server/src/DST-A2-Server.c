#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close */
#include <signal.h>

void MulticastSaleItems();
void SendMCMessage(char message[]);
void CreateBindMCSocket();
void AssignMCNetworkAddresses();

#define SERVER_PORT 7150
#define MAX_MSG 200

int sd, rc, i;
unsigned char ttl = 1;
struct sockaddr_in cliAddr, servAddr;
struct hostent *h;

char *filePath;

char multicastGroup[] = "226.0.10.1";
char TESTmessage[] = "this is a test.";
char HEADER_SALE[] = "SAL";
char HEADER_BID[] = "BID";
char HEADER_FINISHED[] = "FIN";
char FOOTER[] = "DST";

int TIMER_INTERVAL = 5;

int main(int argc, char *argv[])
{
	//set handler for timer(alarm) interupt
	signal(SIGALRM, MulticastSaleItems);

	filePath = argv[0];

	AssignMCNetworkAddresses();

	CreateBindMCSocket();

	alarm(TIMER_INTERVAL);

	while(1)
	{
		//TCP Listener
	}

	/* close socket and exit */
	close(sd);
	exit(0);
}

void AssignMCNetworkAddresses()
{
	/* get host from provided dot format multicast address */
	h = gethostbyname(multicastGroup);
	if (h == NULL )
	{
		printf("%s : unknown host '%s'\n", filePath, multicastGroup);
		exit(1);
	}

	servAddr.sin_family = h->h_addrtype;
	memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
	servAddr.sin_port = htons(SERVER_PORT);

	/* check if dest address is multicast */
	if (!IN_MULTICAST(ntohl(servAddr.sin_addr.s_addr)))
	{
		printf("%s : given address '%s' is not multicast \n", filePath,
				inet_ntoa(servAddr.sin_addr));
		exit(1);
	}
}

void CreateBindMCSocket()
{
	/* create socket */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0)
	{
		printf("%s : cannot open socket\n", filePath);
		exit(1);
	}

	/* bind any port number */
	cliAddr.sin_family = AF_INET;
	cliAddr.sin_addr.s_addr = htonl(INADDR_ANY );
	cliAddr.sin_port = htons(0);
	if (bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr)) < 0)
	{
		perror("bind");
		exit(1);
	}

	if (setsockopt(sd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
	{
		printf("%s : cannot set ttl = %d \n", filePath, ttl);
		exit(1);
	}
}

void SendMCMessage(char message[])
{
	printf("%s : sending data on multicast group '%s' (%s)\n", filePath,
			h->h_name, inet_ntoa(*(struct in_addr *) h->h_addr_list[0]));

	//send message
	rc = sendto(sd, message, strlen(message) + 1, 0,
			(struct sockaddr *) &servAddr, sizeof(servAddr));

	//check message was sent
	if (rc < 0)
	{
		printf("%s : cannot send data\n", filePath);
		close(sd);
		exit(1);
	}
}

void MulticastSaleItems()
{
	//fork here
	//child sends and closes
	//parent returns to previous task
	SendMCMessage(TESTmessage);

	alarm(TIMER_INTERVAL);
}
