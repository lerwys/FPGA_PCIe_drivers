#include <sys/ioctl.h> 
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<fcntl.h>
#include <unistd.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>


#include "../pcie_ioctl.h"



int main()
{	
	int offset=0;
	int j;
	int plik=open("/dev/fpga_pcie", O_RDWR);
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
	printf("Zmapowalem pod adres: %x\n",(unsigned int)devm);
	fflush(stdout);
	int wyjscie=0;
	int i=0;
	char tmp='X';

	//		for(i=0;i<1024*1024;i++){
	//		printf("DMACOUNT\n");
	if(ioctl(plik,SET_DMACOUNT,1)<0){
		printf("Blad przy ioctl\n");
	}


	//First transfer
	if(ioctl(plik,START_STREAM)<0){
		printf("Blad przy ioctl\n");
	}

	for(i=0;i<=10;i++){
		tmp='X';
		if(ioctl(plik,NEXT_STREAM, &offset)<0){
			printf("Blad przy ioctl\n");
		}
		//Print only first char from transfer
		printf("'%c'", devm[offset]);
		printf("\n->");


		//print all data in transfer
		{
			printf("[");
			for(j=0;j<16*1024;j++){
				printf("%c", devm[offset+j-1]);
			}

			printf("]");
		}
		//print changes
		{
			tmp='X';

			for(j=0;j<16*1024;j++){
				if (tmp!=devm[offset+j]){
					printf("%d", j);
					printf("%c\n", devm[offset+j-1]);
					tmp=devm[offset+j];
				}
			}
		}
		//single transfer ended
		printf("DONE\n");
	}


	//Ending stream
	if(ioctl(plik,END_STREAM, &offset)<0){
		printf("Blad przy ioctl\n");
	}	

	//print all data in transfer
//	{
//		printf("[");
//		for(j=0;j<16*1024;j++){
//			printf("%c", devm[offset+j-1]);
//		}
//
//		printf("]");
//	}
	//print changes
//	{
//		tmp='X';
//
//		for(j=0;j<16*1024;j++){
//			if (tmp!=devm[offset+j]){
//				printf("%d", j);
//				printf("%c\n", devm[offset+j-1]);
//				tmp=devm[offset+j];
//			}
//		}
//	}
	//single transfer ended
	printf("DONE ENDING STREAM\n");

	close(plik);
	return 0;
}
