/* 
 *Jacek Kołodziejski jkolodz2@stud.elka.pw.edu.pl
 *Program do dystrybucji danych przez UDP
 *
 * Diagnostic code has been commented
 * UDP transfer base on http://www.abc.se/~m6695/udp.html
 *
 * */


#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h> 
#include <stdlib.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>


#include "../pcie_ioctl.h"

#define BUFLEN 16*1024
#define NPACK 64*1024
#define PORT 9930
#define SRV_IP "192.168.1.10"
/* diep(), #includes and #defines like in the server */

void diep(char *s)
{
	printf("%s\n",s);
	//	return 1;
	exit (1);
}


int main(void)
{
	struct sockaddr_in si_other;
	int s, r, o,p, i, slen=sizeof(si_other);
	int plik;
	unsigned char buf[BUFLEN];
	unsigned char tmp[3];
	int offset=0;

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		diep("socket");

	if ((r=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		diep("socket");
	if ((p=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		diep("socket");
	if ((o=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		diep("socket");




	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	if (inet_aton(SRV_IP, &si_other.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		return(1);
		//		exit(1);
	}

	plik=open("/dev/fpga_pcie", O_RDWR);
	if(plik==-1)
	{
		printf("Nie moge otworzyc urzadzenia!!!\n");
		fflush(stdout);
		exit(1);
	}
	fflush(stdout);

	unsigned char volatile * devm = (unsigned char *) 
		mmap(0,32*1024+1,PROT_READ ,MAP_SHARED,
				plik,0x0000000);
	if(devm == (void *) -1l)
	{
		perror("Nie moge zmapowac pamieci!!!\n");
		exit(1);
	}
	//Xprintf("Zmapowalem pod adres: %x\n",(unsigned int)devm);
	fflush(stdout);
	if(ioctl(plik,SET_DMACOUNT,1)<0){
		printf("Blad przy ioctl\n");
	}

	if(ioctl(plik,START_STREAM)<0){
		printf("Blad przy ioctl\n");
	}

	for (i=1; i<NPACK; i++) {
		//Xprintf("Sending packet %d\n", i);
		//s//Xprintf(buf, "This is packet %d\n", i);

		if(ioctl(plik,NEXT_STREAM, &offset)<0){
			printf("Blad przy ioctl\n");
		}

		//Xprintf("%d\t%c\n", offset,(devm+offset)[0]);
		if(i%1024==0)printf("%d\n", i);
		//Xprintf("Next:");

		//		if (sendto(s, devm+offset, BUFLEN, 0, &si_other, slen)==-1)
		if (sendto(r, (const void *)devm+offset,BUFLEN/4, 0, (const struct sockaddr *)&si_other, slen)==-1)
			diep("sendto1()");
//		if (recvfrom(r, tmp,3*sizeof(char) , 0, &si_other, &slen)==-1)
//			diep("recvfrom()");
		//Xprintf("Received packet from %s:%d\nData: %s\n END\n",inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), tmp);

		if (sendto(s, (const void *)devm+offset+ BUFLEN/4,BUFLEN/4, 0, (const struct sockaddr *)&si_other, slen)==-1)
			diep("sendto2()");
	//	if (recvfrom(s, tmp,3*sizeof(char) , 0, &si_other, &slen)==-1)
//			diep("recvfrom()");
		//Xprintf("Received packet from %s:%d\nData: %s\n END\n",inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), tmp);


		if (sendto(o, (const void *)devm+offset+ BUFLEN/2,BUFLEN/4, 0, (const struct sockaddr *)&si_other, slen)==-1)
			diep("sendto3()");
	//	if (recvfrom(s, tmp,3*sizeof(char) , 0, &si_other, &slen)==-1)
//			diep("recvfrom()");
		//Xprintf("Received packet from %s:%d\nData: %s\n END\n",inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), tmp);

		if (sendto(p, (const void *)devm+offset+ BUFLEN*3/4,BUFLEN/4, 0, (const struct sockaddr *)&si_other, slen)==-1)
			diep("sendto4()");
	//	if (recvfrom(s, tmp,3*sizeof(char) , 0, &si_other, &slen)==-1)
//			diep("recvfrom()");
		//Xprintf("Received packet from %s:%d\nData: %s\n END\n",inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), tmp);

	}


	if(ioctl(plik,END_STREAM, &offset)<0){
		printf("Blad przy ioctl\n");
	}	

	//Xprintf("%d\t%c\n", offset,(devm+offset)[0]);
	if (sendto(s, (const void *)devm+offset,	BUFLEN/4, 0, (const struct sockaddr *)&si_other, slen)==-1)
		diep("sendto1()");
	//if (recvfrom(s, tmp,3*sizeof(char) , 0, &si_other, &slen)==-1)
	//	diep("recvfrom()");
	//Xprintf("Received packet from %s:%d\nData: %s\n END\n",inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), tmp);


	if (sendto(s, (const void *)devm+offset+ BUFLEN/4,BUFLEN/4, 0, (const struct sockaddr *)&si_other, slen)==-1)
		diep("sendto2()");
	//if (recvfrom(s, tmp,3*sizeof(char) , 0, &si_other, &slen)==-1)
//		diep("recvfrom()");
	//Xprintf("Received packet from %s:%d\nData: %s\n END\n",inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), tmp);

	if (sendto(s, (const void *)devm+offset+ BUFLEN/2,BUFLEN/4, 0, (const struct sockaddr *)&si_other, slen)==-1)
		diep("sendto3()");
	//if (recvfrom(s, tmp,3*sizeof(char) , 0, &si_other, &slen)==-1)
//		diep("recvfrom()");
	//Xprintf("Received packet from %s:%d\nData: %s\n END\n",inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), tmp);

	if (sendto(s, (const void *)devm+offset+ BUFLEN*3/4,BUFLEN/4, 0, (const struct sockaddr *)&si_other, slen)==-1)
		diep("sendto4()");
	//if (recvfrom(s, tmp,3*sizeof(char) , 0, &si_other, &slen)==-1)
//		diep("recvfrom()");
	//Xprintf("Received packet from %s:%d\nData: %s\n END\n",inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), tmp);




	close(plik);
	close(s);
	return 0;
}

