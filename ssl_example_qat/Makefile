all: client server

qat: clientqat serverqat

client: Client.c transfer.h
	gcc -Wall -o client  Client.c -L/usr/lib -lssl -lcrypto

server: Server.c transfer.h
	gcc -Wall -o server  Server.c -L/usr/lib -lssl -lcrypto

# don't forget where your non default SSL is:
# setenv LD_LIBRARY_PATH /usr/local/ssl/lib/
# export LD_LIBRARY_PATH=/usr/local/ssl/lib/
clientqat: Client.c transfer.h
	gcc -Wall -o client  Client.c -I/usr/local/ssl/include -L/usr/local/ssl/lib -lssl -lcrypto
	gcc -Wall -o clientqat  Client.c -DQAT_INTEGRATION -I/usr/local/ssl/include -L/usr/local/ssl/lib -lssl -lcrypto

serverqat: Server.c transfer.h
	gcc -Wall -o server  Server.c -I/usr/local/ssl/include -L/usr/local/ssl/lib -lssl -lcrypto
	gcc -Wall -o serverqat  Server.c -DQAT_INTEGRATION -I/usr/local/ssl/include -L/usr/local/ssl/lib -lssl -lcrypto

clean:
	rm -f client server clientqat serverqat *~
