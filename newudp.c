#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>


// static long int MAX_QUEUE_LIMIT = 1000;
static long int MAX_BUFFER_SIZE = 999999;

// Terminates the proxy indicating error
void closeWithError(char *error);


// Function accepts client socket and exhchanges data with the client and the server
void exchangeDataWithClient(int client_socket);


int
main(int argc, char **argv)
{

   //Initialize the data
	char buffer[MAX_BUFFER_SIZE];
	struct hostent *host_entity;
	int listenfd;
	struct sockaddr_in  cliaddr, servaddr;

	memset(buffer,0,MAX_BUFFER_SIZE);

	//Reset the client address
	bzero(&cliaddr, sizeof(cliaddr));

	if(argc != 5)
	{
		printf("ERROR,- one of the following parameters may be missing\n\t<hostname>\t<portno>\t<File path>\t<Delay> \n");
		exit(1);
	}
	else
	{
		printf("HOST ADDRESS = %s\n",argv[1]);
		printf("PORT NO.= %s\n",argv[2]);
		printf("DELAY = %s\n",argv[3]);
		printf("FILE PATH= %s\n",argv[4]);

	}

    // Open a socket
	listenfd = socket(AF_INET, SOCK_DGRAM, 0);

    // Setup Proxy address
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(atoi(argv[2]));

    //Bind the Socket
	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		closeWithError("Bind Error for the request!");
	}


	if ((argv[1][0] >= 'a' && argv[1][0] <= 'z') ||  (argv[1][0] >= 'A' && argv[1][0] <= 'z'))
	{
		printf("Parsing for host name:%s\n", argv[1]);
		if ((host_entity = gethostbyname(argv[1])) != NULL)
		{
			memcpy(&cliaddr.sin_addr, (struct in_addr*)host_entity->h_addr,host_entity->h_length);
		}
		else {
			printf("Could not resolve host.\n");
			return -1;
		}

		printf("s_addr = %s\n", inet_ntoa((struct in_addr)cliaddr.sin_addr));
	}
	else {
		printf("Parsing for IP:%s\n", argv[1]);
		if(inet_aton(argv[1],&cliaddr.sin_addr) == 0){
			printf("Could not resolve host.\n");
			return -1;
		}
	}

	//Set the port
	cliaddr.sin_port = htons(atoi(argv[2]));

   

	int fd;
   if ((fd = open(argv[4],O_RDONLY)) < 0)
   	{
   		closeWithError(" ERROR can't open file!\n");
   	}

   size_t nbytes = 1024;

	if (connect(listenfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) == -1)
	{
		printf("Could not connect to web.\n");
		exit(-1);
	}

	printf("Delay:%ld\n", (long)atoi(argv[3])*1000);

   // nbytes=sizeof(buffer);
   int bytes_read = read(fd,buffer,nbytes);
   do
   	{
	    usleep(atoi(argv[3])*1000);                         


   		if (bytes_read == -1)
   		{
   			return -1;
   		}
   		// printf("Reading bytes:%d\n", bytes_read);
   		if(sendto(listenfd,buffer,nbytes,0,(struct sockaddr*)&cliaddr, sizeof(cliaddr)) != -1)
	   {
	   	// printf("sending\n");
	   }
		memset(buffer,0,MAX_BUFFER_SIZE);

   	}
   	while((bytes_read = read(fd,buffer,nbytes)) != 0);

   	close(listenfd);

   	printf("Finished Sending.\n");
	return 0;
}


void closeWithError(char *error) {

	printf("%s\n",error);
	exit(1);
}
