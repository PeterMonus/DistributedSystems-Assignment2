#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close */
#include <signal.h>

#define SERVER_PORT_MC 7150
#define SERVER_PORT_TCP 7151
#define MAX_MSG 200

void ReadUserDetails();
void InteruptHandler();
void CreateMCNetworkAddresses();
void CreateBindMCSockect();
void JoinMulticastGroup();
void RecieveMC();
void TCPopen_connect();
void TCPsendMessage(char message[]);
void TCPcloseConnection();

/*MC variables*/
int sd, rc, n, cliLen;
struct ip_mreq mreq;
struct sockaddr_in cliAddr, servAddr;
struct in_addr mcastAddr;
struct hostent *h;
char multicastGroup[] = "226.0.10.1";
/*TCP variables*/
char TCP_buff[MAX_MSG];
struct sockaddr_in TCP_ServerAddr;
char TCP_IP[] = "127.0.0.1";
int TCPsock;

/*logic*/
char msg[MAX_MSG];
char userName[40];
char *filePath;

int main(int argc, char *argv[])
{
	filePath = argv[0];

	ReadUserDetails();
	//set handler for timer(alarm) interupt
	signal(SIGINT, InteruptHandler);

	CreateMCNetworkAddresses();
	CreateBindMCSockect();
	JoinMulticastGroup();
	RecieveMC();

	return 0;

}

void ReadUserDetails()
{
	printf("Name: ");
	scanf("%s", userName);
	printf("Welcome %s \n", userName);
}

void InteruptHandler()
{
	int proccessID = fork();
	if (proccessID == 0)
	{
		TCPopen_connect();
		TCPsendMessage("this is a test TCP message.");
		TCPcloseConnection();

		exit(0);
	}
	else
		return;
}

void CreateMCNetworkAddresses()
{
	/* get mcast address to listen to */
	h = gethostbyname(multicastGroup);
	if (h == NULL )
	{
		printf("%s : unknown group '%s'\n", filePath, multicastGroup);
		exit(1);
	}

	memcpy(&mcastAddr, h->h_addr_list[0], h->h_length);

	/* check given address is multicast */
	if (!IN_MULTICAST(ntohl(mcastAddr.s_addr)))
	{
		printf("%s : given address '%s' is not multicast\n", filePath,
				inet_ntoa(mcastAddr));
		exit(1);
	}
}

void CreateBindMCSockect()
{
	/* create socket */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0)
	{
		printf("%s : cannot create socket\n", filePath);
		exit(1);
	}

	/* bind port */
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY );
	servAddr.sin_port = htons(SERVER_PORT_MC);
	if (bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
	{
		printf("%s : cannot bind port %d \n", filePath, SERVER_PORT_MC);
		exit(1);
	}
}

void JoinMulticastGroup()
{
	/* join multicast group */
	mreq.imr_multiaddr.s_addr = mcastAddr.s_addr;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY );

	rc = setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &mreq,
			sizeof(mreq));
	if (rc < 0)
	{
		printf("%s : cannot join multicast group '%s'", filePath,
				inet_ntoa(mcastAddr));
		exit(1);
	}
}

void RecieveMC()
{
	printf("%s : listening to mgroup %s:%d\n", filePath, inet_ntoa(mcastAddr),
			SERVER_PORT_MC);

	/* infinite receive loop */
	while (1)
	{
		cliLen = sizeof(cliAddr);
		n = recvfrom(sd, msg, MAX_MSG, 0, (struct sockaddr *) &cliAddr,
				&cliLen);
		if (n < 1)
		{
			printf("%s : cannot receive data\n", filePath);
			continue;
		}

		printf("%s : from %s:%d on %s : %s\n", filePath,
				inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port),
				multicastGroup, msg);
	}/* end of infinite receive loop */
}

void TCPopen_connect()
{
	//create chunck of memory for server ip address
	memset(&TCP_ServerAddr, 0, sizeof(TCP_ServerAddr));
	//set required fields of server ip address
	TCP_ServerAddr.sin_family = AF_INET;
	TCP_ServerAddr.sin_port = htons(SERVER_PORT_TCP);
	inet_pton(AF_INET, TCP_IP, &TCP_ServerAddr.sin_addr);

	//create socket for connecting to server
	TCPsock = socket(AF_INET, SOCK_STREAM, 0);

	connect(TCPsock, (struct sockaddr*) &TCP_ServerAddr,
			sizeof(TCP_ServerAddr));
}

void TCPsendMessage(char message[])
{
	int bytesSent;
	bytesSent = write(TCPsock, message, strlen(message));
	if (bytesSent < 1)
	{
		printf("Message was not sent.");
	}
}

void TCPcloseConnection()
{
	close(TCPsock);
}

