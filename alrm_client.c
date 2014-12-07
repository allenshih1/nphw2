#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<errno.h>

#define MAXLINE 1024

static void sig_alrm(int);

void dg_cli(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen)
{
	int n;
	char sendline[MAXLINE+1], recvline[MAXLINE+1];
	struct sigaction sa;
	struct msghdr s_msg, r_msg;
	struct iovec s_iov[2], r_iov[2];
	struct myhdr {
		int seq;
		int last;
	} s_hdr, r_hdr;

	connect(sockfd, (struct sockaddr *) pservaddr, servlen);

	{
		s_msg.msg_name = NULL;
		s_msg.msg_namelen = 0;
		s_msg.msg_iov = s_iov;
		s_msg.msg_iovlen = 2;
		s_msg.msg_control = NULL;
		s_msg.msg_controllen = 0;
		s_msg.msg_flags = 0;

		s_iov[0].iov_base = &s_hdr;
		s_iov[0].iov_len = sizeof(struct myhdr);
		s_iov[1].iov_base = sendline;
		s_iov[1].iov_len = MAXLINE;

		r_msg.msg_name = NULL;
		r_msg.msg_namelen = 0;
		r_msg.msg_iov = r_iov;
		r_msg.msg_iovlen = 1;
		r_msg.msg_control = NULL;
		r_msg.msg_controllen = 0;
		r_msg.msg_flags = 0;

		r_iov[0].iov_base = &r_hdr;
		r_iov[0].iov_len = sizeof(struct myhdr);
	}

	s_hdr.seq = 0;
	s_hdr.last = 0;
	sa.sa_handler = sig_alrm;
	sa.sa_flags = 0;
	sigaction(SIGALRM, &sa, NULL);

	while(1)
	{
		n = fread(sendline, 1, MAXLINE, fp);
		s_iov[1].iov_len = n;
		if(feof(fp)){
			s_hdr.last = 1;
		}
		if(ferror(fp)){
			fprintf(stderr, "file error\n");
			exit(1);
		}

		do
		{
			sendmsg(sockfd, &s_msg, 0);

			alarm(1);
			if( (n = recvmsg(sockfd, &r_msg, 0)) < 0) {
				if(errno == EINTR) {
					/*fprintf(stderr, "timeout\n");*/
				} else {
					perror("Receive error");
					exit(1);
				}
			} else if(s_hdr.seq == r_hdr.seq) {
				alarm(0);
				s_hdr.seq++;
				break;
			}
		} while(1);

		if(s_hdr.last == 1)
			break;
	}
}

static void sig_alrm(int signo)
{
	return;
}

int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in servaddr;

	if(argc != 3)
	{
		fprintf(stderr, "usage: %s <ip> <port>", argv[0]);
		exit(1);
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	dg_cli(stdin, sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	return 0;
}
