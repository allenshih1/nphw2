#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define MAXLINE 1024

void dg_recv(FILE *fp, int sockfd, struct sockaddr *pcliaddr, socklen_t clilen)
{
	int n;
	socklen_t len;
	char mesg[MAXLINE+1];
	struct msghdr msg;
	struct iovec iov[2];
	struct myhdr {
		int seq;
		int last;
	} hdr;
	int nextseq;

	{
		msg.msg_name = pcliaddr;
		msg.msg_namelen = clilen;
		msg.msg_iov = iov;
		msg.msg_iovlen = 2;
		msg.msg_control = NULL;
		msg.msg_controllen = 0;
		msg.msg_flags = 0;

		iov[0].iov_base = &hdr;
		iov[0].iov_len = sizeof(struct myhdr);
		iov[1].iov_base = mesg;
		iov[1].iov_len = MAXLINE;
	}

	nextseq = 0;

	for(;;)
	{
		msg.msg_namelen = clilen;
		msg.msg_iovlen = 2;
		iov[1].iov_len = MAXLINE;
		n = recvmsg(sockfd, &msg, 0);

		if(hdr.seq == nextseq) {
			fwrite(mesg, n - sizeof(struct myhdr), 1, fp);
			nextseq++;
		}

		msg.msg_iovlen = 1;
		sendmsg(sockfd, &msg, 0);
		if(hdr.last == 1)
			break;
	}
}

int main(int argc, char **argv)
{
	int sockfd;
	FILE *fp;
	struct sockaddr_in servaddr, cliaddr;

	if(argc != 3)
	{
		fprintf(stderr, "usage: %s <port> <output>\n", argv[0]);
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[1]));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	if( (fp = fopen(argv[2], "w")) == 0 )
	{
		fprintf(stderr, "Cannot open file %s\n", argv[2]);
		exit(1);
	}

	dg_recv(fp, sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr));

	fclose(fp);

	return 0;
}
