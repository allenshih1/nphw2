all: alrm_client sel_client opt_client server

alrm_client: alrm_client.c
	gcc -o alrm_client alrm_client.c

sel_client: sel_client.c
	gcc -o sel_client sel_client.c

opt_client: opt_client.c
	gcc -o opt_client opt_client.c

server: server.c
	gcc -o server server.c
