all: alrm_client sel_client opt_client alrm_server sel_server opt_server

alrm_client: alrm_client.c
	gcc -o alrm_client alrm_client.c

sel_client: sel_client.c
	gcc -o sel_client sel_client.c

opt_client: opt_client.c
	gcc -o opt_client opt_client.c

alrm_server: alrm_server.c
	gcc -o alrm_server alrm_server.c

sel_server: sel_server.c
	gcc -o sel_server sel_server.c

opt_server: opt_server.c
	gcc -o opt_server opt_server.c
