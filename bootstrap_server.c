#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<string.h>
#include<time.h>
#include<netdb.h>
#include<fcntl.h>
#include<errno.h>

#define ERROR -1
#define MAX_CLIENTS 5
#define MAX_DATA 1024
#define HOSTNAME "nunki.usc.edu"
#define PEER 2
#define PROVIDER 1

int checkClient(char *data);
int validateProvider(char *data);
void processProvider(struct sockaddr_in provider, int provider_sock);

int main(int argc, char** argv){

	//Server Socket and client socket
	int server_sock,client_sock;

	//Server address & Client Address
	struct sockaddr_in server,client;

	//Hostent
	struct hostent *hp = gethostbyname(HOSTNAME);

	//To store Length of structures
	socklen_t len;

	//Number of bytes sent
	int sent;
	
	static int no_of_peers=0;

	//Data
	char data[MAX_DATA],*result;
	int data_len,i;

	//Operations
	char *sendData;
	int sendDataLen;

	char *ip=NULL;
	
	//File operation variables
	FILE *fp;
	char fileName[] = "directory.txt";
	
	//Create Socket
	server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	if(server_sock == ERROR){
		perror("Socket: ");
		exit(1);
	}
	
	int flag = 1;
	int sockProp = setsockopt(server_sock,SOL_SOCKET,SO_REUSEADDR,(void *)&flag,sizeof(int));
	
	if(sockProp < 0){
		perror("sockProp Error:");
		exit(1);
	}
	
	//Initialize Server connection variables for binding
	server.sin_family = AF_INET;
	server.sin_port = htons(3995);

	if (hp == 0) {
		fprintf(stderr, "%s: unknown host",HOSTNAME);
		exit(2);
	}
	bcopy(hp->h_addr_list[0], &server.sin_addr, hp->h_length);

	//0-padding
	//	bzero(&server.sin_zero,8);
	len = sizeof(struct sockaddr_in);

	//Bind Server to the port

	if(bind(server_sock,(struct sockaddr *) &server,len) == ERROR){
		perror("bind:");
		exit(1);
	}

	if(open(fileName,O_EXCL) != -1){ 
		remove(fileName);
	}

	fp = fopen(fileName,"w");
	if(fp == NULL){
		fprintf(stderr, "Can't open input file\n");
		exit(1);
	}
	
	//i = 0;
	//Keep listening
	while(1) {
		//If client request found then accept
		//printf("Server with IP: %s listening on port %d\n",inet_ntoa(server.sin_addr), ntohs(server.sin_port));

		if(recvfrom(server_sock, data, MAX_DATA, 0,(struct sockaddr *) &client, &len) == -1){
			//perror("Receive Error:");
			exit(1);
		}else{
			switch(fork()){
			case -1:
				perror("Fork Error:");
				//close(client_sock);
				exit(1);
			case 0: 
				//close(server_sock);
				data_len = strlen(data);

				if (data_len > 0) {
					strcat(data,"\n");
					if(checkClient(data) == PEER){

						fprintf(fp,data);
						fclose(fp);
						printf("Peer %c Registration Successfull\n\n", data[4]);
						printf("Peer %c\n%d\n",data[4], strcmp(data[4],"4"));
							
					}else if(checkClient(data) == PROVIDER){
						printf("Incoming Message From Provider%c\n",data[8]);
						if(validateProvider(data)){
							printf("Provider%c Logged In Successfully\n",data[8]);
							processProvider(client, server_sock);						
						}
					}
				}
				close(client_sock);
				break;
			default:
				//close(client_sock);
				continue;
			}
		}
	}

	printf("Server Disconnected\n\n");	
	close(server_sock);
	return 0;
}

/*
@author: Saketkumar Srivastav
@description: This method verifies processes the content provider
*/

void processProvider(struct sockaddr_in provider, int provider_sock){
	//printf("Provider IP: %s\n port %d\n",inet_ntoa(provider.sin_addr), ntohs(provider.sin_port));
	
	char input[MAX_DATA];
	sprintf(input, "directory.txt");
	if(sendto(provider_sock, input, MAX_DATA, 0, &provider, sizeof(struct sockaddr_in))==-1){
		perror("Send Error:");
		exit(1);
	}else{
		printf("Filename Sent\n\n");
	}
}

/*
@author: Saketkumar Srivastav
@description: This method verifies whether the provider has correct credentials
*/

int validateProvider(char *data){
	int validated = 0;
	
	char *uname = NULL;
	char *pass = NULL;
	
	uname = strstr(data,"provider");
	pass = strstr(data,"Spring12");
	
	if(uname == NULL || pass == NULL){
		validated = 0;
	}else if(strlen(uname) > 0 && strlen(pass) > 0){
		validated = 1;
	}else{
		validated = 0;
	}
	
	return validated;
}

/*
@author: Saketkumar Srivastav
@description: This method checks whether the request is from peer or provider
*/
int checkClient(char *data){	
	int client = 0;
	char *p = NULL,*q = NULL;
	
	p = strstr(data,"Peer");
	q = strstr(data,"Provider");
	
	if(p == NULL && q == NULL){
		client = 0;
	}else if(q == NULL && strlen(p) > 0){
		client = PEER;
	}else if(p == NULL && strlen(q) > 0){
		client = PROVIDER;		
	}else{
		client = 0;
	}
	
	return client;		
}
