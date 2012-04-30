#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<strings.h>
#include<arpa/inet.h>
#include<netdb.h>

#define ERROR	-1
#define BUFFER	1024
#define UDP_PORT 3995
#define HOSTNAME "nunki.usc.edu"

void connectToPeer(char *filename, int groupNumber);
int countPeers(char *filename, int groupNumber);
char *returnFirstPeer(char *filename, int groupNumber);
char* getOtherPeerData(char *filename, char *peer, int groupNumber);
char* returnPeerData(char *peerName);

int main(int argc, char** argv){
	struct sockaddr_in remote_server,client_addr;

	//Hostent
	struct hostent *hp = gethostbyname(HOSTNAME);

	int sock;
	char input[BUFFER], data[BUFFER];
	int len,addrlen;
	char *ip=NULL;
	int i,j,members = 0;
	
	int pid;
		
	remote_server.sin_family = AF_INET;
	remote_server.sin_port = htons(UDP_PORT);

	if (hp == 0) {
		fprintf(stderr, "%s: unknown host",HOSTNAME);
		exit(-1);
	}
	bcopy(hp->h_addr_list[0], &remote_server.sin_addr, hp->h_length);

	
	for (i = 0; i < 2; i++) {
		pid = fork();

		if(pid == -1){
			perror("Client Fork Error:");
			close(sock);
			exit(1);
		}

		if(pid == 0){
			socklen_t clientSockLength = sizeof(struct sockaddr_in);
			sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

			if(sock == ERROR)
			{
				perror("Client Socket:");
				exit(1);
			}
					
			sprintf(input, "Provider%d provider Spring12",i+1);

			if(sendto(sock, input, BUFFER, 0, &remote_server, clientSockLength)==-1){
				perror("Send Error:");
				exit(1);
			}
			
			if(recvfrom(sock, data, BUFFER, 0,(struct sockaddr *) &remote_server, &len) == -1){
				perror("Receive Error:");
				exit(1);
			}
			
			members = countPeers(data, i+1);
			printf("\nGroup -> %d has %d peers.\n",i+1,members);
			
			connectToPeer(data, i+1);
			
			//printf("Closing Content Provider UDP\n");
			close(sock);
			exit(0);
		}else{
			//wait(&j);
			if(i == 1){
				break;
				
			}else{
				continue;
			}
		}
	}
	//close(sock);
	exit(0);
}

char *returnFirstPeer(char *filename, int groupNumber){
	FILE *fp;
	char *firstPeerEntry = NULL;
	char ch;
	
	if(groupNumber == 1){
		ch = '1';
	}else if(groupNumber == 2){
		ch = '2';
	}
	fp = fopen(filename, "r");
	if ( fp != NULL ){
	      char line [ 33 ];
	      int index = 0; 
	      while ( fgets ( line, sizeof line, fp ) != NULL ) /* read a line */
	      {
	      	 index = strlen(line) - 2;
		 if(line[index] == ch){
		 	//Important because on returning string pointer the stack gets deallocated hence use Heap
		 	firstPeerEntry = malloc(sizeof(char *) * 30);	 	
			strcpy(firstPeerEntry,line);
			break;
		 }	 
	      }

	      fclose ( fp );
	}
	else{
	      perror ( "File problem" ); /* why didn't the file open? */
	}	
	
	return firstPeerEntry;
}

void connectToPeer(char *filename, int groupNumber){

	struct sockaddr_in client_addr_tcp,peer_addr_tcp,temp;
	int tcpSock;
	
	char *peerName = NULL;
	char *peerIp = NULL;
	char *peerPort = NULL;
	char *peerDetails = NULL;
	char *result = NULL;
	
	char input[BUFFER];
	
	int i,peerPort_int;
	socklen_t clientSockLength = sizeof(struct sockaddr_in);
	
	peerDetails = returnFirstPeer(filename,groupNumber);
	result = strtok( peerDetails, " " );
	for(i = 0; result != NULL;i++ ) {
	    
	    if(i == 0){
	    	peerName = result;
	    }else if(i == 1){
		peerIp = result;
		
	    }else if(i == 2){
		peerPort = result;
	    }else if(i == 3){
	    
	    }
	    result = strtok( NULL, " " );
	}	
	
	peerPort_int = atoi(peerPort);
	
	//Preparing Peer Connection
	peer_addr_tcp.sin_family = AF_INET;
	peer_addr_tcp.sin_port = htons(peerPort_int);
	//strcpy(&peer_addr_tcp.sin_addr,peerIp);

	struct hostent *hp = gethostbyname(HOSTNAME);
	bcopy(hp->h_addr_list[0], &peer_addr_tcp.sin_addr, hp->h_length);

	//TCP connection	
	tcpSock = socket(AF_INET, SOCK_STREAM, 0);
	
	if(tcpSock == ERROR){
		perror("Client TCP Socket:");
		exit(1);
	}
	
	if((connect(tcpSock, (struct sockaddr *)&peer_addr_tcp, sizeof(struct sockaddr_in))) == ERROR)
	{
		perror("connect:");
		exit(-1);
	}
	
	if(getsockname(tcpSock,(struct sockaddr *)&temp, &clientSockLength) == ERROR){
		perror("Client TCP GetSockName:");
		exit(1);
	}

	//printf("\nProvider IP:%s & Provider port %d\nPeer IP: %s & Peer Port: %d\n",inet_ntoa(temp.sin_addr),ntohs(temp.sin_port),inet_ntoa(peer_addr_tcp.sin_addr),ntohs(peer_addr_tcp.sin_port));	
		
	int length = read(tcpSock, input, BUFFER);
		
	if(length > 0){
		//printf("%s joined Group%d\n",peerName,groupNumber);
		sprintf(input, "%s joined Group%d\n",peerName,groupNumber);
		int sendBytes = write(tcpSock, input, BUFFER);
		sendBytes = 0;
		
		char *otherPeerData = NULL;
		sprintf(input, "Provider%d ",groupNumber);
		otherPeerData = getOtherPeerData(filename, peerName, groupNumber);
		strcat(input,otherPeerData);
		sendBytes = write(tcpSock, input, BUFFER);
		if(sendBytes > 0){
			//printf("Second Send: %s\n",input);
			
			length = read(tcpSock,input,BUFFER);
			if(length > 0){
				//printf("Received ACK from %s\n",peerName);
				//printf("%s\n",input);
			}else{
				printf("ACK not received from %s\n",peerName);
			}
		}
		
	}else{
		printf("Nothing Received\n\n");
	}
	
	//printf("Closing Provider TCP\n");
	close(tcpSock);
}

int countPeers(char *filename, int groupNumber){

	FILE *fp;
	int counter = 0, index;
	char ch;
	
	if(groupNumber == 1){
		ch = '1';
	}else if(groupNumber == 2){
		ch = '2';
	}
	fp = fopen(filename, "r");
	
	if ( fp != NULL ){
	      char line [ 33 ];
		
	      while ( fgets ( line, sizeof line, fp ) != NULL ) /* read a line */
	      {
	      	 index = strlen(line) - 2;
		 if(line[index] == ch){
			counter++;		 
		 }	 
	      }
	      fclose ( fp );
	}
	else{
	      perror ( "File problem" ); /* why didn't the file open? */
	}	
	
	return counter;
}


char* getOtherPeerData(char *filename, char *headPeer, int groupNumber){	
	FILE *fp;
	char ch;
        char line [ 33 ];
	char *peerData = NULL;
	char *resultString = NULL;
	
	resultString = malloc(sizeof(char *) * BUFFER);
	
	if(groupNumber == 1){
		ch = '1';
	}else if(groupNumber == 2){
		ch = '2';
	}
	fp = fopen(filename, "r");
	if ( fp != NULL ){

	      int index = 0; 

	      while (fgets ( line, sizeof line, fp ) != NULL ) /* read a line */
	      {
	      	 index = strlen(line) - 2;
	      	 if(line[index] == ch){
	      	 	//printf("HeadPeer: %s\nLine: %s\n",headPeer,line);
	      	 	if(headPeer[4] != line[4]){
   	 		      	peerData = returnPeerData(line);	
	      	 		strcat(resultString, peerData);
	      	 		strcat(resultString," ");
//	      	 		strcat(resultString," ");
//				printf("Result String: %s\n",resultString);
	      	 	}	
	      	 }
	      }

	      fclose ( fp );
	}
	else{
	      perror ( "File problem" ); /* why didn't the file open? */
	}	
	
	return resultString;		
}

char* returnPeerData(char *peerDetails){

	char *peerName = NULL;
	char *peerPort = NULL;
	char *peerData = NULL;
	char *results = NULL;
	char *sep = " "; 
	int i = 0;
	
	peerData = malloc(sizeof(char *) * BUFFER);
//	printf("PeerDetails: %s\n",peerDetails);
	results = strtok( peerDetails, " " );
	for(i = 0; results != NULL;i++ ) {
	    
	    if(i == 0){
	    	peerName = results;
	    	strcat(peerData,peerName);
	    }else if(i == 2){
		peerPort = results;
		strcat(peerData,sep);
		strcat(peerData,peerPort);
	    }
	    results = strtok( NULL, " " );
	}	
	
	return peerData;
}
