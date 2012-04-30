gcc -o bootstrap_server bootstrap_server.c -lsocket -lnsl -lresolv
gcc -o content_provider content_provider.c -lsocket -lnsl -lresolv
gcc -o peer peer.c -lsocket -lnsl -lresolv
