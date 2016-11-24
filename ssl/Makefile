CC=gcc

#SSLINC= -I/home/kujawa/devel/syseng/openssl/include
#SSLLIB= -L/home/kujawa/devel/syseng/openssl -lssl -lcrypto
SSLLIB=-lssl -lcrypto

ssl_server:	ssl_server.o
	$(CC) ssl_server.o $(SSLLIB) -lpthread -lsocket -lnsl -o ssl_server

ssl_server.o:	ssl_server.c defs.h
	$(CC) -Wall -c $(SSLINC) ssl_server.c 

defs.h:	../queue.h ../bbs.h
	./mkdefs.sh

clean:
	rm -f defs.h ssl_server *.o *~
