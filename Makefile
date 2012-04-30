all: bootstrap provider peer

bootstrap:bootstrap_server.c
	gcc -o bootstrap_server bootstrap_server.c -lsocket -lnsl -lresolv

provider:content_provider.c
	gcc -o content_provider content_provider.c -lsocket -lnsl -lresolv
	
peer:peer.c
	gcc -o peer peer.c -lsocket -lnsl -lresolv
