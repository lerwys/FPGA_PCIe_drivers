#define MAGIC_FPGA_PCIE 'K'

//------------------IOCTL CALL NAMES------------------
//send initiator reset
#define RESET_CARD 	_IO(MAGIC_FPGA_PCIE,0)

//start data stream
#define	START_STREAM	_IO(MAGIC_FPGA_PCIE,1)

//get next piece of data (PING-PONG data transfer)
#define	NEXT_STREAM	_IOR(MAGIC_FPGA_PCIE,2, int)

//end stream
#define	END_STREAM	_IOR(MAGIC_FPGA_PCIE,3, int)

//set the ammount of dma's to request
#define SET_DMACOUNT	_IOW(MAGIC_FPGA_PCIE,4,int)

//set the tlp size
#define SET_TLPSIZE	_IOW(MAGIC_FPGA_PCIE,5,int)

//set the ammount of tlps in one dma 
#define SET_TLPCOUNT	_IOW(MAGIC_FPGA_PCIE,6,int)

//memset a char over the writebuffer
#define MEMSET_WRITEBUFFER _IOW(MAGIC_FPGA_PCIE,7,char)

