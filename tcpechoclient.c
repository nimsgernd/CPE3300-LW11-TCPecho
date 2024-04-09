// Simple TCP echo server

#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>  
#include <string.h>	
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>  


// Max message to echo
#define MAX_MESSAGE	1000

#define LAB_BROADCAST   (in_addr_t) 0xC0A818FF
#define HOME_BROADCAST  (in_addr_t) 0xC0A801FF

/* server main routine */

int main(int argc, char** argv)
{

	// locals
	unsigned short port = 3300; // default port
    struct sockaddr_in receiverIP;
	int connection;

	int sock; // socket descriptor

	// Was help requested?  Print usage statement
	if (argc > 1 && ((!strcmp(argv[1],"-?"))||(!strcmp(argv[1],"-h"))))
	{
		printf("\nUsage: tcpechoserver [-p port] port is the requested \
 port that the server monitors.  If no port is provided, the server \
 listens on port 3300.\n\n");
		exit(0);
	}

    // get the IP address from ARGV
	if (argc > 1 && !strcmp(argv[1],"-ip"))
	{
		if (!inet_pton(AF_INET, argv[2], &(receiverIP.sin_addr)))
		{
			perror("Error parsing port option");
			exit(0);
		}
	}
	
	// get the port from ARGV
	if (argc > 3 && !strcmp(argv[3],"-p"))
	{
		if (sscanf(argv[4],"%hu",&port)!=1)
		{
			perror("Error parsing port option");
			exit(0);
		}
	}
	
	// ready to go
	printf("tcp echo server configuring on port: %d\n",port);

	receiverIP.sin_family = AF_INET;
	receiverIP.sin_port = htons(port);
	
	// for TCP, we want IP protocol domain (PF_INET)
	// and TCP transport type (SOCK_STREAM)
	// no alternate protocol - 0, since we have already specified IP
	
	if ((sock = socket( PF_INET, SOCK_STREAM, 0 )) < 0) 
	{
		perror("Error on socket creation");
		exit(1);
	}
  
  	// lose the pesky "Address already in use" error message
	int yes = 1;

	if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) 
	{
		perror("setsockopt");
		exit(1);
	}

	// establish address - this is the server and will
	// only be listening on the specified port
	struct sockaddr_in sock_address;
	
	// address family is AF_INET
	// our IP address is INADDR_ANY (any of our IP addresses)
    // the port number is per default or option above

	sock_address.sin_family = AF_INET;
	sock_address.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_address.sin_port = htons(port);

	if (connect(sock, (struct sockaddr*) &receiverIP, sizeof(receiverIP)) < 0)
	{
		perror("Error calling connect");
		exit(-1);
	}

	
	// go into forever loop and echo whatever message is received
	// to console and back to source
	char* buffer = calloc(MAX_MESSAGE,sizeof(char));
	int buffer_len;
	int bytes_sent;
	int echoed;
	
    while (1)
	{			
		fgets(buffer,MAX_MESSAGE,stdin);

		buffer_len = strlen(buffer) + 1;

		// send it back to client
		if ( (bytes_sent = write(sock, buffer, buffer_len + 1)) < 0 )
		{
			perror("Error sending msg");
			exit(-1);
		}
		else
		{			
			printf("Bytes sent: %d\n",bytes_sent);
		}

		echoed = read(sock, buffer, MAX_MESSAGE-1);

		if (echoed == 0)
		{	// socket closed
			printf("====Client Disconnected====\n");
			close(connection);
			break;  // break the inner while loop
		}

		// make sure buffer has null terminator
		buffer[echoed] = '\0';

		// see if client wants us to disconnect
		if (strncmp(buffer,"quit",4)==0)
		{
			printf("====Server Disconnecting====\n");
			close(connection);
			break;  // break the inner while loop
		}

		// print info to console
		printf("Received message\n");

		// put message to console
		printf("Message: %s From: %s %d\n", buffer,
		       inet_ntoa(receiverIP.sin_addr),
			   ntohs(receiverIP.sin_port));	
    }

	free(buffer);
	// will never get here
	return(0);
}
