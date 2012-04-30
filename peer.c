#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<strings.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<netinet/tcp.h>
#include<time.h>

#define ERROR	-1
#define BUFFER	1024
#define UDP_PORT 3995
#define HOSTNAME "nunki.usc.edu"

struct Peer{
	char *name;
	int port;
	int groupNumber;
};

void acceptTCPConnection(int tcpSock, struct sockaddr_in client_addr_tcp);
//void connectToOtherPeers(char *data);
void decodeAndConnectToOtherPeer(char *peerDetails);
//int getPeerCount(char *data);
void initializePeer(struct Peer *peer);
void connectToPeer(char *name, int port,int groupNumber);

int main(int argc, char** argv){
	struct sockaddr_in remote_server,client_addr,client_addr_tcp;

	//Hostent
	struct hostent *hp = gethostbyname(HOSTNAME);

	int sock,tcpSock,client_sock;
	char input[BUFFER];
	int len,addrlen;
	char *ip=NULL;
	int i,j;
	
	int pid;
	//Initializing server variables
	//printf("%s:\n",ntohs(remote_server.sin_port));
	
	remote_server.sin_family = AF_INET;
	remote_server.sin_port = htons(UDP_PORT);

	if (hp == 0) {
		fprintf(stderr, "%s: unknown host",HOSTNAME);
		exit(-1);
	}
	bcopy(hp->h_addr_list[0], &remote_server.sin_addr, hp->h_length);
	
	for (i = 0; i < 4; i++) {
		pid = fork();
		srand(time(NULL) ^ (getpid() << 16));
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
			
			//Peer TCP PORT generation
			client_addr_tcp.sin_family = AF_INET;
			client_addr_tcp.sin_port = 0;

			bcopy(hp->h_addr_list[0], &client_addr_tcp.sin_addr, hp->h_length);
			tcpSock = socket(AF_INET, SOCK_STREAM, 0);
			
			/*int flag = 1;
			int sockProp = setsockopt(tcpSock, IPPROTO_TCP,TCP_NODELAY,(char *)&flag,sizeof(int));
			if(sockProp < 0){
				perror("sockProp Error:");
				exit(1);
			}*/
			
			if(tcpSock == ERROR){
				perror("Client TCP Socket:");
				exit(1);
			}
			
			if((bind(tcpSock,(struct sockaddr *) &client_addr_tcp, clientSockLength)) == ERROR){
				perror("Client TCP Bind:");
				exit(1);
			}
		
			if(getsockname(tcpSock,(struct sockaddr *)&client_addr_tcp,&clientSockLength) == ERROR){
				perror("Client TCP GetSockName:");
				exit(1);
			}
			
			srand(time(NULL) ^ (getpid() << 16));
		
			//rand()%4+1
			
			sprintf(input, "Peer%d %s %d group%d",i+1, inet_ntoa(client_addr_tcp.sin_addr), ntohs(client_addr_tcp.sin_port),rand()%2+1);

			if(sendto(sock, input, BUFFER, 0, &remote_server, sizeof(struct sockaddr_in))==-1){
				perror("Send Error:");
				exit(1);
			}
			printf("%s\n",input);
			//printf("---------Peer %d---------\n",i);
			acceptTCPConnection(tcpSock,client_addr_tcp);
			
			close(tcpSock);					
			close(sock);
			exit(0);
		}else{
			//wait(&j);
			continue;
		}
	}
}

void acceptTCPConnection(int tcpSock, struct sockaddr_in client_addr_tcp){
	struct sockaddr_in peer_addr_tcp,temp;
	int client_sock;
	int data_len = NULL;
	char data[BUFFER];
	socklen_t clientSockLength = sizeof(struct sockaddr_in);
		
	//printf("Listening...\n");
	//Server Listening at port

	if(listen(tcpSock,5) == ERROR){
		perror("Listen:");
		exit(1);
	}
	
	client_sock = accept(tcpSock,(struct sockaddr *) &client_addr_tcp, &clientSockLength);
	
	if(client_sock == ERROR){
		perror("Accept Peer:");
		exit(1);
	}else{
		
		if(getsockname(tcpSock,(struct sockaddr *)&peer_addr_tcp, &clientSockLength) == ERROR){
			perror("Client TCP GetSockName:");
			exit(1);
		}
		
		//printf("\nClient IP:%s & Client port %d\nPeer IP: %s & Peer Port: %d\n",inet_ntoa(client_addr_tcp.sin_addr),ntohs(client_addr_tcp.sin_port),inet_ntoa(peer_addr_tcp.sin_addr),ntohs(peer_addr_tcp.sin_port));	
		//Sending ACK to provider
		strcpy(data, "1");

		int sendBytes = write(client_sock,data,BUFFER);
		char ack[3];
		char *msgFromPro = NULL;
		char *data1 = NULL;
		if(sendBytes > 0){
			//Receive Welcome Message
			data_len = read(client_sock,data,BUFFER);
			//msgFromPro = data;
			printf("\n%s",data);
			if(data_len > 0){
				//printf("data_len : %d\n",data_len);
				//printf("Received Data: %s\n",data);

				char *test = NULL;

				char *prov = NULL;
				prov = "Provider";
				
				//Getting other peer information
				data_len = read(client_sock,data,BUFFER);
				
				//printf("Data 2: %s\n",data);	
				test = strstr(data,prov);

				if(test != NULL){//From Provider	
					//printf("Message From Provider: %s\n",data);
					//printf("Provider: %s\n",data);
					strcpy(ack,"ACK->1");
					sendBytes = write(client_sock,ack,BUFFER);
					
					//Connect To Other Peers
					decodeAndConnectToOtherPeer(data);
					//printf("Decode Done\n");
						
				}else{//From Peer
					//printf("In Peer\n");
					data_len = read(client_sock,data,BUFFER);
					//printf("Peer: %s\n",data);
					exit(0);
				}
			}else{
					printf("No Data Received\n");
			}
		}else{
			printf("Nothing Sent");
		}               
	}
}

void initializePeer(struct Peer *peer){
	peer->name = NULL;
	peer->port = 0;
	peer->groupNumber = 0;
}
/*
int getPeerCount(char *data){
	char *result = NULL;
	int i = 0,peerCount = 0;
	
	result = strtok( data, " " );
	for(i = 0,peerCount = 0; result != NULL;i++ ) {
		peerCount++;
	    result = strtok( NULL, " " );
	}
	
	printf("Peer Count: %d\n",i-1);
	return peerCount;
}
*/
void decodeAndConnectToOtherPeer(char *peerDetails){

	struct Peer peer[3];
	char *result = NULL;
	int i = 0,peerCount;
	
	char groupNumber = "";
	int groupNumber_int = 0;
	
	//Extract Group Number from The Data
	groupNumber = peerDetails[8];
	groupNumber_int = atoi(&groupNumber);
		
	result = strtok( peerDetails, " " );
	
	for(i = 0,peerCount=0; result != NULL;i++ ) {
		if(i >= 1){
			if(result != NULL){
				if(i%2 == 1){
					initializePeer(&peer[peerCount]);
				    	peer[peerCount].name = result;
				    	//printf("Peer[%d]: Name-> %s\n",peerCount,peer[peerCount].name);
				}else if(i%2 == 0){
					peer[peerCount].port = atoi(result);
					if(groupNumber_int == 1 || groupNumber_int == 2){
						peer[peerCount].groupNumber = groupNumber_int;
					}
					
					//printf("Peer[%d]: Port-> %d\n",peerCount,peer[peerCount].port);
					peerCount++;
			    	}
			}
	    	}
	    	result = strtok( NULL, " " );
	}
	
	//Connecting To Other Peers
	for(i = 0; i < peerCount;i++){
		connectToPeer(peer[i].name,peer[i].port,peer[i].groupNumber);
	}	
}

void connectToPeer(char *name, int port,int groupNumber){

	struct sockaddr_in remote_peer,temp;

	//Hostent
	struct hostent *hp = gethostbyname(HOSTNAME);
	socklen_t clientSockLength = sizeof(struct sockaddr_in);
	
	int peerSock;
	char input[BUFFER];
	int len,addrlen;	
	
	remote_peer.sin_family = AF_INET;
	remote_peer.sin_port = htons(port);

	if (hp == 0) {
		fprintf(stderr, "%s: unknown host",HOSTNAME);
		exit(-1);
	}
	bcopy(hp->h_addr_list[0], &remote_peer.sin_addr, hp->h_length);
	
	//TCP connection	
	peerSock = socket(AF_INET, SOCK_STREAM, 0);
	
	if(peerSock == ERROR){
		perror("Client TCP Socket:");
		exit(1);
	}
	
	if((connect(peerSock, (struct sockaddr *)&remote_peer, sizeof(struct sockaddr_in))) == ERROR)
	{
		perror("connect:");
		exit(-1);
	}
	
	if(getsockname(peerSock,(struct sockaddr *)&temp, &clientSockLength) == ERROR){
		perror("Client TCP GetSockName:");
		exit(1);
	}
	//printf("\nProvider IP:%s & Provider port %d\nPeer IP: %s & Peer Port: %d\n",inet_ntoa(temp.sin_addr),ntohs(temp.sin_port),inet_ntoa(peer_addr_tcp.sin_addr),ntohs(peer_addr_tcp.sin_port));	
	sprintf(input,"%s joined group %d\n",name,groupNumber);
	int length = write(peerSock, input, BUFFER);
	
}
