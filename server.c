/* PROGETTO C 
    Leonardo
    Colosi
    1799057
    colosi.1799057@studenti.uniroma1.it
*/


#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#define BUFF_SIZE 256
#define PORT_NUM 5000
#define bzero(b,len) (memset((b), '\0', (len)), (void)0)


void error(char *msg) {
	perror(msg);
	exit(1);
}

void makeGraph(char *input, int graph[BUFF_SIZE][BUFF_SIZE]);                   

int ctrlMsg(char *msg, int buff_size, int num_of_node, int *p1, int *p2);                  

int minPath(int D[], bool N[], int size);                                                                             

char* dijkstra( int graph[BUFF_SIZE][BUFF_SIZE], int src, int dst, int size, int *res);     


int main (int argc, char *argv[]) {
    /* Inizzailizzazione delle variabili e della matrice */

    char file_buf[BUFF_SIZE];
    int num_of_node = 0;
    int graph[BUFF_SIZE][BUFF_SIZE];
    int n1=-1, n2=-1, res=-1;
    char asw[BUFF_SIZE];
    int ctrl = -1;

    for (int i=0; i<BUFF_SIZE; i++)
        for (int j=0; j<BUFF_SIZE; j++)
            graph[i][j] = 0;

    /* Lettura del file rete.txt */
    
    printf("Reading Network graph... \n\n");
    FILE *fp = fopen("rete.txt", "r");
	if (fp != NULL) {
        while(fgets(file_buf, sizeof(file_buf), fp) != NULL) {
            makeGraph(file_buf, graph);
            num_of_node++;
            printf("%s", file_buf);
        }
		fclose(fp);
	} else 
		printf("Error opening file\n");

    printf("\nwating for client... \n");
    
        
    /* Creazione soket TCP lato server */
    
	int sockfd, newsockfd, portno, clilen;
	char buffer[BUFF_SIZE];
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
		error("ERROR opening socket");
	
	bzero ((char*) &serv_addr, sizeof(serv_addr));
	portno = PORT_NUM;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

	if (newsockfd < 0)
		error("ERROR on accept");

   
    /* In attesa della connesione TCP */
	
	while (1) {
		bzero(buffer, BUFF_SIZE);
		n = read(newsockfd,buffer,BUFF_SIZE-1);
		
        if (n < 0) 
			error("ERROR reading from socket");
    
		if (n < 0) 	error("ERROR writing to socket");

        /* Operazioni del Server */
 
        if (strncmp("min:",buffer,4) == 0) {
            /* Controllo del formato del messaggio */

            int e = ctrlMsg(buffer, strlen(buffer), num_of_node, &n1, &n2);

            if (e == -1)
                n = write(newsockfd,"ERR - Formato non corretto ",28);
            else if (e == -2)
                n = write(newsockfd,"Uno o entrambi i nodi non esistono",34);
            
            /* Calcolo del percorso minimo */
            
            else {
                char *bar = dijkstra(graph, n1, n2, num_of_node, &res);  
                                            
                if (res == INT_MAX )
                    n = write(newsockfd,"percorso non esistente ",25);
                else {
                    sprintf(asw,"Il percorso minimo tra %d e %d e' [%s] con costo %d", n1, n2, bar, res);
                    free(bar);
                    n = write(newsockfd, asw, strlen(asw));
                    printf("\nI got your message\n");
                    printf("wating for client... \n");
                }
            }
		}
        else if (strcmp(buffer, "exit\n") == 0) {
            /* Chiusura della socket */

    		n = write(newsockfd,"\nBye\n",4);
			if (n < 0)  error("ERROR writing to socket");
            printf("\nEND\n");
			close(newsockfd);
			exit(0);

		} else { 
    		n = write(newsockfd,"ERR - Formato non corretto ",28);
		}
	}
	close(sockfd);
    return 0; 
}


void makeGraph(char *input, int graph[BUFF_SIZE][BUFF_SIZE]){
    /* La funzione estrae dal grafo nel file rete.txt i pesi degli archi 
        inserendoli in una matrice n*n con n = numero dei nodi. 
          Se due nodi n1, n2 non sono connessi il peso sara' grafo[n1][n2]=0 */
    
    int i = -1, k = 0;
    char tmp[20]; 

    while (input[++i] != ':' && input[i] != '\0')
        tmp[k++] = input[i];                                   

    tmp[k] = '\0';
    int index = atoi(tmp);
    k = 0;
    int j = i;

    while (++i < strlen(input)-1){
        int bar = -1;                                   

        while(input[++j] != ',' && input[j] != '\0')
            tmp[k++] = input[j];

        tmp[k] = '\0';
        bar = atoi(tmp);
        k = 0;        

        while(input[++j] != ' ' && input[j] != '\0')
            tmp[k++] = input[j];
        
        tmp[k] = '\0';
        graph[index][bar] = atoi(tmp);                            
        k = 0;
        i = j;

    }
}


int ctrlMsg(char *msg, int buff_size, int num_of_node, int *p1, int *p2){
    /* La funzione estrare dalla stringa messaggio: ID_NODO_SRC, ID_NODO_DEST 
        e ne controlla l'apparteneza all'insieme dei nodi di rete.txt 
         se il formato del msg e' sbagliato ritorna -1 
          se uno dei due nodi non esite ritorna -2 */

    msg[buff_size-1]='\0';
    char *l = (char *) malloc(sizeof(char) * buff_size+1);
    char *r = (char *) malloc(sizeof(char) * buff_size+1);
    int j=0, i=4, a=0;
   
    /* controllo che la prima parte del msg da "min:" sia un numero */    

    do{
        if (msg[i] < '0' || msg[i] > '9'){
            if(msg[i] == '-') return -2;
            return -1; 
        }
        l[j++] = msg[i++];
    } while (msg[i] != ',' && msg[i] != '\0');
   
    l[j] = '\0';
    if (msg[i] != ',') return -1;
    i++;
    j=0;

    /* controllo che la seconda parte del msg da "min:ID_1" sia un numero  */

    do{
        if (msg[i] < '0' || msg[i] > '9'){
            if (msg[i] == '-') return -2;
            return -1;
        }
        r[j++] = msg[i++];
    } while (msg[i] != '\0');
    
    r[j] = '\0';

    *p1 = atoi(l);
    *p2 = atoi(r);

    free(l);
    free(r);

    /* controllo se n1 e n2 sono nella lista di nodi */

    if (*p1 > num_of_node || *p2 > num_of_node)                 
        return -2;
    return 0;
}
 

int minPath(int D[], bool N[], int size) {
    /* La funzione calcola il minimo tra gli elementi non in N */    

    int min = INT_MAX, min_index;
    for (int v = 0; v < size; v++){
        if (N[v] == false && D[v] <= min){
            min = D[v], min_index = v;
        }
    }
    return min_index;
}


char* dijkstra( int graph[BUFF_SIZE][BUFF_SIZE], int src, int dst, int size, int *res){
    /* La Funzione applica l'algoritmo di Dijkstra al grafo estratto dal file e 
        restituisce un array con il percorso di peso minimo e il peso di tale percorso */
                                                                       
    char *foo = (char *)malloc(sizeof(char) * size); 
    char *bar = (char *)malloc(sizeof(char) * size);
    int path[size];
    int P[size];
    int D[size];
    bool N[size];
    
    /* Inizializzazione dei pesi sui collegamenti */
    
    for (int i = 0; i < size; i++){
        if (graph[src][i] == 0){
            D[i] = INT_MAX;
            P[i] = 0;
        }
        else{
            D[i] = graph[src][i];
            P[i] = src;                  
        }
        N[i] = false;
    }
    P[src] = -1;
    D[src] = 0;
    N[src] = true;

    /* Analisi dei vicini e aggionamento del vettore delle distanze */

    for (int count = 0; count < size - 1; count++) {
        int u = minPath(D, N, size);             
        N[u] = true; 
        for (int v = 0; v < size; v++) { 
            if (!N[v] && graph[u][v] && D[u] != INT_MAX && D[u] + graph[u][v] < D[v]){
                P[v] = u;                                                
                D[v] = D[u] + graph[u][v];
            }
        }
    }

    int tmp = dst;
    int i = 0; path[i]=dst;                                 

    /* Calcolo del percorso da src a dst tramite il vettore dei padri */

    do{
        if(P[tmp] == -1) P[tmp] = src;
        path[++i] = (P[tmp]);                               
        tmp = P[tmp];
    } while (tmp>0 && tmp != src);

    for (int j=i; j>=0; j--){
        snprintf(foo, size,"%d, ", path[j]);
        strcat(bar,foo);
    }
    bar[strlen(bar)-2] = '\0';    
    free(foo);
    *res = D[dst];
    return bar;
}

