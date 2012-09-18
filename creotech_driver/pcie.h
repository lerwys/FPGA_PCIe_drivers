#define SUCCESS 0
#define DEVICE_NAME "fpga_pcie"
#define DEV_N "[FPGA_PCIE] "
#define BUF_SIZE                  (16*1024)

//XAPP 1052 registers
//check xapp1052.pdf appendix for tables and reference

//000H	- Device Control Status Register 	(DCSR)	 	(R/W)
#define 	REG_DCSR	0x000
#define 	BIT_INIT	0		//1 bit	(RW)
#define 	BIT_INITRESET	1		//1 bit	(RW)
//1:7 	- reserved
#define		BIT_VERSIONNUM	(1<<8)		//8bit	(R0)
#define		BIT_CIDATAWIDTH	(1<<16)		//4bit	(R0)
//20:23 - reserved
#define		BIT_FPGAFAMILY	(1<<24)		//8bit	(R0)

//004H	- Device DMA Control Status Register	(DDMACR)	(R/W)
#define		REG_DDMACR	0x004	
#define 	BIT_WDMASTART	1		//1bit	(RW)
//1:4	- reserved
#define		BIT_WDMARELORD	(1<<5)		//1bit	(RW)
#define		BIT_WDMANOSNOOP	(1<<6)		//1bit	(RW)
#define		BIT_WDMAINTDONE (1<<7)		//1bit	(RW)
#define		BIT_WDMADONE	(1<<8)		//1bit	(RW)
//9:15	- reserved
#define		BIT_RDMASTART	(1<<16)		//1bit	(RW)
//17:22 - reserved
#define		BIT_RDMARELORD	(1<<21)		//1bit	(RW)
#define		BIT_RDMANOSNOOP	(1<<22)		//1bit	(RW)
#define		BIT_RDMAINTDONE	(1<<23)		//1bit	(RW)
#define		BIT_RDMADONE	(1<<24)		//1bit	(RW)
//25:30	- reserved
#define		BIT_RDMADATAERR	(1<<31)		//1bit	(RW)

//008H 	- Write DMA TLP Address			(WDMATLPA)	(R/W)
#define		REG_WDMATLPA	0x008		//30bit	(RW)
//0:1	- reserved

//00cH	- Write DMA TLP Size			(WDMATLPS)	(R/W)
#define		REG_WDMATLPS	0x00c
#define		BIT_WDMATLPS	0		//13bit	(RW)
//13:15	- reserved
#define		BIT_WDMATLPTC	(1<<16)		//3bit	(RW)
#define		BIT_64BITWENABLE (1<<19)	//1bit	(RW)
//20:23	- reserved
#define		BIT_WDMAUTLPA	(1<<24)		//8bit	(RW)

//010H	- Write DMA TLP Count			(WDMATLPC)	(R/W)
#define		REG_WDMATLPC	0x010		//16bit	(RW)
//16:31	- reserved

//014H	- Write DMA Data Pattern		(WDMATLPP)	(R/W)
#define		REG_WDMATLPP	0x014		//32bit (RW)

//018H	- Read DMA Expected Data Pattern	(RDMATLPP)	(R/W)
#define		REG_RDMATLPP	0x018		//32bit	(RW)

//01cH	- Read DMA TLP Address			(RDMATLPA)	(R/W)
#define		REG_RDMATLPA	0x01c		//30bit (RW)
//0:1	- reserved

//020H	- Read DMA TLP Size			(RDMATLPS)	(R/W)
#define		REG_RDMATLPS	0x020
#define		BIT_RDMATLPS	0		//13bit	(RW)
//13:15	- reserved
#define		BIT_RDMATLPTC	(1<<16)		//3bit	(RW)
#define		BIT_64BITRENABLE (1<<19)	//1bit	(RW)
//20:23	- reserved
#define		BIT_RDMAUTLPA	(1<<24)		//8bit	(RW)

//024H	- Read DMA TLP Count			(RDMATLPC)	(R/W)
#define		REG_RDMATLPC	0x024		//16bit (RW)
//16:31	- reserved

//028H	- Write DMA Performance			(WDMAPERF)	(R0)
#define		REG_WDMAPERF	0x028		//32bit	(R0)

//02cH	- Read DMA Performance			(RDMAPERF)	(R0)
#define		REG_RDMAPERF	0x02c		//32bit (R0)

//030H	- Read DMA Status			(RDMASTAT)	(R0)
#define		REG_RDMASTAT	0x030
#define		BIT_COMPLREC	0		//8bit	(R0)
#define		BIT_COMPLTAG	(1<<8)		//8bit	(R0)
//16:31 - reserved

//034H	- Number of Read Completion w/Data	(NRDCOMP)	(R0)
#define		REG_NRDCOMP	0x034		//32bit	(R0)

//038H	- Read Completition Data Size		(RCOMPDSIZW)	(R0)
#define		REG_RCOMPDSIZW	0x038		//32bit	(R0)

//03cH	- Device Link Width Status		(DLWSTAT)	(R0)
#define		REG_DLWSTAT	0x03c
#define 	BIT_CAPMAXLWIDTH 0		//6bit	(R0)
//6:7	- reserved
#define		BIT_NEGMAXLWIDTH (1<<8)		//6bit	(R0)
//14:31 - reserved

//040H	- Device Link Transaction Size Status 	(DLTRSSTAT)	(R0)
#define		REG_DLTRSSTAT	0x040
#define		BIT_CAPMAXPSIZE 0		//3bit	(R0)
//3:7	- reserved
#define		BIT_PROMAXPSIZE	(1<<8)		//3bit	(R0)
//11:15	- reserved
#define		BIT_RREQSIZE	(1<<16)		//3bit	(R0)
//19:31 - reserved

//044H	- Device Miscellaneous Control		(DMISCCONT)	(RW)
#define		REG_DMISCCONT	0x044
//9:31	- reserved
#define 	BIT_RECNPOSTOK	(1<<8)		//1bit	(RW)
//2:7	- reserved
#define		BIT_RMETEN	(1<<1)		//1bit	(RW)
#define		BIT_COMPLSTREN	0		//1bit	(RW)

#ifdef VERBOSE
#define PRINT1(z) printk(z)
#define PRINT2(y,z) printk(y,z)
#define PRINT3(x,y,z) printk(x,y,z)
#define PRINT4(w,x,y,z) printk(w,x,y,z)
#define PRINT5(v,w,x,y,z) printk(v,w,x,y,z)
#else
#define PRINT1(z)  
#define PRINT2(y,z)  
#define PRINT3(x,y,z)  
#define PRINT4(w,x,y,z)  
#define PRINT5(v,w,x,y,z)  
#endif


void vma_open(struct vm_area_struct *vma);
void vma_close(struct vm_area_struct *vma);
int init_char_device(void);
void exit_char_device(void);
int fpga_probe (struct pci_dev *dev, const struct pci_device_id *id);   
void fpga_remove (struct pci_dev *dev); 
static void fpga_exit(void);
static int fpga_init(void);
void fpga_init_device(void);
static int fpga_char_open(struct inode *, struct file *);
static int fpga_char_release(struct inode *, struct file *);
static int fpga_char_mmap(struct file *file, struct vm_area_struct *vma);
long fpga_char_ioctl(struct file * file, unsigned int cmd, unsigned long arg);
static ssize_t fpga_char_read(struct file *, char *, size_t, loff_t *);
static ssize_t fpga_char_write(struct file *, const char *, size_t, loff_t *);
irqreturn_t fpga_irq_handler(int irq, void *dev_id);


DEFINE_PCI_DEVICE_TABLE(id_table)=
{
	{ PCI_DEVICE(0x10EE,0x0007) },
	{}
};


struct pcie_descriptor_structure {
	unsigned long base_start;
	unsigned long base_end;
	unsigned long base_len;
	void __iomem * region;
	int irq;
	int irq_pin;
	char * buffer;
	bool even_buffer;
	dma_addr_t	buffer_hw_addr;
	dma_addr_t	offset;
	dma_addr_t	dma_addr;
	wait_queue_head_t       irq_wait;
	int dma_done;
	int tlp_size;
	int tlp_count;
	int dma_count;
	bool wake;
	u32 pattern;
} pcie_descr;

struct pci_dev * gDev= NULL;

struct pci_driver fpga_pcie_driver = {
name: 			"fpga_pcie",
			id_table:		id_table,   /* must be non-NULL for probe to be called */
			probe:			fpga_probe,
			remove:			fpga_remove,
};

static struct file_operations fops = {
	.read = fpga_char_read,
	.write = fpga_char_write,
	.open = fpga_char_open,
	.release = fpga_char_release,
	.unlocked_ioctl = fpga_char_ioctl,
	.mmap = fpga_char_mmap
};

static struct vm_operations_struct fpga_char_vm_ops = {
	.open = vma_open,
	.close = vma_close
};

static int Device_Open = 0;	  

static struct miscdevice fpga_dev = {
	MISC_DYNAMIC_MINOR,
	DEVICE_NAME,
	&fops
};

void 	write_reg (u32 offset, u32 val)
{
	writel(val, (pcie_descr.region + offset));
}

u32 read_reg (u32 offset)
{
	u32 ret = 0;
	ret = readl(pcie_descr.region + offset);  
	return ret; 
}

