/*
 * Jacek Kołodziejski jkolodz2@stud.elka.pw.edu.pl
 *
 * Program odbierający dane poza systemem
 *
 * based on http://www.abc.se/~m6695/udp.html
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFLEN 16*1024
#define NPACK 64*1024
#define PORT 9930

void diep(char *s)
{
	perror(s);
	exit(1);
}

int main(void)
{
	struct sockaddr_in si_me, si_other;
	int s,r,o,p, i, slen=sizeof(si_other);
	unsigned char buf[BUFLEN+1];

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	    diep("socket");
	if ((r=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	    diep("socket");
	if ((p=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	    diep("socket");
	if ((o=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	    diep("socket");


	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(s, &si_me, sizeof(si_me))==-1)
		diep("bind");

	for (i=0; i<NPACK; i++) {
		if (recvfrom(s, buf, BUFLEN, 0, &si_other, &slen)==-1)
			diep("recvfrom()");
		if (recvfrom(r, buf, BUFLEN, 0, &si_other, &slen)==-1)
			diep("recvfrom()");
		if (recvfrom(o, buf, BUFLEN, 0, &si_other, &slen)==-1)
			diep("recvfrom()");
		if (recvfrom(p, buf, BUFLEN, 0, &si_other, &slen)==-1)
			diep("recvfrom()");

		buf[BUFLEN]='\0';
		if(i%1024==0)printf("%d\n", i);
		//		printf("Received packet from %s:%d\nData: %c\n END\n", 
		//				inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf[0]);
		//	if (sendto(s, (const void *)"OK\0",3*sizeof(char), 0, (const struct sockaddr *)&si_other, slen)==-1)
		//		diep("sendto()");
	}

	close(s);
	return 0;
}

