#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<errno.h>

#define MAXLINE 1024

int readable_timeo(int fd, int sec);

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
			
			if(readable_timeo(sockfd, 1) == 0) {
				/* time out */
			} else if( (n = recvmsg(sockfd, &r_msg, 0)) < 0) {
					perror("Receive error");
					exit(1);
			} else if(s_hdr.seq == r_hdr.seq) {
				s_hdr.seq++;
				break;
			}
		} while(1);

		if(s_hdr.last == 1)
			break;
	}
}

int readable_timeo(int fd, int sec)
{
	fd_set rset;
	struct timeval tv;

	FD_ZERO(&rset);
	FD_SET(fd, &rset);

	tv.tv_sec = sec;
	tv.tv_usec = 0;

	return (select(fd+1, &rset, NULL, NULL, &tv));
}

int main(int argc, char **argv)
{
	int sockfd;
	FILE *fp;
	struct sockaddr_in servaddr;

	if(argc != 4)
	{
		fprintf(stderr, "usage: %s <ip> <port> <input>\n", argv[0]);
		exit(1);
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	if( (fp = fopen(argv[3], "r")) == 0 )
	{
		fprintf(stderr, "Cannot open file %s\n", argv[3]);
		exit(1);
	}

	dg_cli(fp, sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	fclose(fp);

	return 0;
}
