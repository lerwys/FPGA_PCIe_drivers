#include "lib/pciDriver.h"

#define MAX 16

void testDevice( int i );
void testDirectIO(pd_device_t *dev);
void testDMA(pd_device_t *dev);

typedef struct {
	unsigned long src_addr_h;
	unsigned long src_addr_l;
	unsigned long dst_addr_h;
	unsigned long dst_addr_l;
	unsigned long length;
	unsigned long control;
	unsigned long next_bda_h;
	unsigned long next_bda_l;
	unsigned long status;
} bda_t;

void write_dma(unsigned long *base, bda_t *dma);

int main() 
{
	int i;
	
	for(i=0;i<4;i++) {
		testDevice( i );
	}

	return 0;
}

void testDevice( int i )
{
	pd_device_t dev;
	int ret;
	
	printf("Trying device %d ...", i);
	ret = pd_open( i, &dev );

	if (ret != 0) {
		printf("failed\n");
		return;
	}

	printf("ok\n");	

	testDirectIO(&dev);
	testDMA(&dev);
	
	pd_close( &dev );
}

void testDirectIO(pd_device_t *dev)
{
	int i,j,ret;
	unsigned long val,buf[MAX];
	unsigned long *bar0, *bar1;

	bar0 = pd_mapBAR( dev, 0 );
	bar1 = pd_mapBAR( dev, 1 );

	/* test register memory */
	bar0[0] = 0x1234565;
	bar0[1] = 0x5aa5c66c;

	for(i=0;i<MAX;i++) {
		val = bar0[i];
		printf("%08x\n",val);
	}		

	/* write block */
	for(i=0;i<MAX;i++) {
		buf[i] = ~i;
	}
	memcpy( (void*)bar0, (void*)buf, MAX*sizeof(unsigned int) );

	/* read block */
	memcpy( (void*)buf, (void*)bar0, MAX*sizeof(unsigned int) );
	for(i=0;i<MAX;i++) {
		val = buf[i];
		printf("%08x\n",val);
	}
	printf("\n\n");

	/* unmap BARs */
	pd_unmapBAR( dev,0, bar0 );
	pd_unmapBAR( dev,1, bar1 );
	
}

void testDMA(pd_device_t *dev)
{
	int i,j,ret;
	unsigned int val;
	unsigned int *bar0, *bar1;
	pd_kmem_t km;
	unsigned int *ptr;
	bda_t dma;
	const unsigned long BASE_DMA_UP   = (0x0C000 >>2);
	const unsigned long BASE_DMA_DOWN = (0x0C040 >>2);	
	const unsigned long BRAM_SIZE = 0x1000;

	bar0 = pd_mapBAR( dev, 0 );
	bar1 = pd_mapBAR( dev, 1 );

	/* test register memory */
	bar0[0] = 0x1234565;
	bar0[1] = 0x5aa5c66c;

	ptr = (unsigned int*)pd_allocKernelMemory( dev, 32768, &km );
	if (ptr == NULL) {
		printf("failed\n");
		pd_unmapBAR( dev,0, bar0 );
		pd_unmapBAR( dev,1, bar1 );
		exit(-1);
	}


	/* Print kernel buffer info */
	printf("Kernel buffer physical address: %lx\n", km.pa);
	printf("Kernel buffer size: %lx\n", km.size);

	/* Reset the DMA channel */
	*(bar1+BASE_DMA_DOWN+0x5) = 0x0000000A;

	/* Fill buffer with zeros */
	memset( ptr, 0, BRAM_SIZE );

	/* Send a DMA transfer */
	dma.src_addr_h = 0x00000000;
	dma.src_addr_l = km.pa;
	dma.dst_addr_h = 0x00000000;
	dma.dst_addr_l = 0x00000000;
	dma.length = BRAM_SIZE;
	dma.control = 0x01000000;		
	dma.next_bda_h = 0x00000000;
	dma.next_bda_l = 0x00000000;

	write_dma(bar1+BASE_DMA_DOWN,&dma);	

	/* wait */
	sleep(5);

	for(i=0;i<BRAM_SIZE/4;i++) {
		val = bar0[i];
		printf("%08x\n",val);
	}		

	ret = pd_freeKernelMemory( &km );

	/* unmap BARs */
	pd_unmapBAR( dev,0,bar0 );
	pd_unmapBAR( dev,1,bar1 );
	
}

void write_dma(unsigned long *base, bda_t *dma) {
		base[0] = dma->src_addr_h;
		base[1] = dma->src_addr_l;
		base[2] = dma->dst_addr_h;
		base[3] = dma->dst_addr_l;
		base[4] = dma->length;
		base[6] = dma->next_bda_h;
		base[7] = dma->next_bda_l;
		base[5] = dma->control;		/* control is written at the end, starts DMA */
}
