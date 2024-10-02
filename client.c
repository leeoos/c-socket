/*CLIENT*/

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define BUFF_SIZE 256
#define PORT_NUM 5000
#define bzero(b,len) (memset((b), '\0', (len)), (void)0)
#define BUFF_SIZE 256

void error(char*msg) {
	perror(msg);
	exit(0);
}


int main () {

    char file_buf[BUFF_SIZE];
    int graph[BUFF_SIZE][BUFF_SIZE];


	int sockfd, portno, n, not_end = 1;

	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[BUFF_SIZE];

	portno = PORT_NUM;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname("leo-K53SD");
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	bzero ((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");
	
    printf("\nIl client, una volta connesso al server può richiedere il cammino di costo minimo tra due nodi nella rete:\n\n");
    
    FILE *fp = fopen("rete.txt", "r");
	if (fp != NULL) {
        while(fgets(file_buf, sizeof(file_buf), fp) != NULL) {
            printf("%s", file_buf);
        }
		fclose(fp);
	} else 
		printf("Error opening file\n");
    
    printf("\n\nUsage:\n");
    printf("\nInput: min:ID_NODO_SRC,ID_NODO_DEST;\n");
    printf("Output : percorso minimo [ID_NODO_SRC,...,ID_NODO_DEST]con costo 'n';\n\n");
    printf("Digitare exit per uscire;\n\n");

	while (not_end) {
		
		printf("Please enter the message: ");
		bzero(buffer, BUFF_SIZE);
		fgets(buffer, BUFF_SIZE-1,stdin);
		n = write(sockfd, buffer, strlen(buffer));
		
		if (n< 0)
			error("ERROR writing to socket");
		if (strcmp("exit\n", buffer) == 0) {
			printf("\nclosing the socket\n");
			not_end = 0;
	    }
        bzero(buffer, BUFF_SIZE);
	    n = read(sockfd, buffer, BUFF_SIZE-1);
	    if (n < 0) 
	    	error("ERROR reading from socket");
	    printf("%s\n\n", buffer);
	}
    close(sockfd);
    return 0;
}	
