all:
	make client
	make server

client:
	gcc client.c -o client -lm -lpthread -lssl -lcrypto -Wall

server:
	gcc server.c -o server -lm -lpthread -lssl -lcrypto -Wall

clean:
	rm -f client server 
	rm -f assembled_files/*