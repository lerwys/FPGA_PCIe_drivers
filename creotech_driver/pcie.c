/*
 * Jacek Kołodziejski jkolodz2@stud.elka.pw.edu.pl
 *
 * PCIe device driver for multichannel GEM detector
 */

#include <linux/kernel.h> /*printk*/
#include <linux/slab.h> /*kmalloc*/
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>  /* udev*/
#include <linux/dma-mapping.h> /* DMA */
#include <linux/interrupt.h> /* IRQ */
#include <asm/system.h> /*cli,flagi*/
#include <asm/uaccess.h> /*copy_from/to_user*/
#include <linux/wait.h>  /* interruptible_wait/ wake_up*/
#include <linux/sched.h> /*TASK_INTERRUPTIBLE*/
#include <linux/ioctl.h>
#include "pcie.h"
#include "pcie_ioctl.h"

MODULE_LICENSE("GPL v2");

/*
 * vma_open - called upon mmap opening
 * @vma_area_struct vma - vma descriptor
 * 
 * does nothing
 */
void vma_open(struct vm_area_struct *vma)
{
	PRINT3(KERN_DEBUG DEV_N "VMA open, virt %lx, phys %lx\n",
			vma->vm_start, vma->vm_pgoff << PAGE_SHIFT);
}

/*
 * vma_close - called upon mmap closing 
 * @vma_area_struct vma - vma descriptor
 * 
 * does nothing
 */
void vma_close(struct vm_area_struct *vma)
{
	PRINT1(KERN_DEBUG DEV_N "VMA close.\n");
}

/*
 * fpga_char_open - fopen
 * @inode: unnused
 * @file: unnused
 *
 * Only one program is allowed to use this device at a time
 */
static int fpga_char_open(struct inode *inode, struct file *file)
{

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	PRINT1(KERN_DEBUG DEV_N  "Device opened\n");
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

/*
 * fpga_char_release - fclose
 * @inode: unnused
 * @file: unnsed
 *
 * The device is now free, and can be accesed by any program
 */
static int fpga_char_release(struct inode *inode, struct file *file)
{
	Device_Open--;
	PRINT1(KERN_DEBUG DEV_N  "Device released\n");
	module_put(THIS_MODULE);

	return 0;
}

/* 
 * fpga_charmmap - mmap handler
 * @file *filp - unnused
 * @vm_area_struct - vma describer
 *
 * Remap the write buffer in to user space. Allows great performace applications.
 */

static int fpga_char_mmap(struct file *filp, struct vm_area_struct *vma)
{
	vma->vm_flags |= VM_LOCKED;
	if (remap_pfn_range(vma, vma->vm_start, pcie_descr.buffer_hw_addr >> PAGE_SHIFT,
				vma->vm_end - vma->vm_start,
				vma->vm_page_prot))
		return -EAGAIN;

	vma->vm_ops = &fpga_char_vm_ops;
	vma_open(vma);
	return 0;
}

/*
 * run_dma - run a single dma request
 * @bool send_interrupt - whether to send an interrupt on completing the task
 * @dma_addr_t offset - the offset to add to the target buffer pointer
 *
 * Sets all the registers and requests a DMA transfer. It may sleep waiting for
 * a device interrupt depending on send_interrupt.
 */

void run_dma(bool send_interrupt, dma_addr_t offset)
{
	pcie_descr.dma_addr=pci_map_single(gDev, pcie_descr.buffer+(!pcie_descr.even_buffer?BUF_SIZE:0)+pcie_descr.offset, BUF_SIZE, DMA_FROM_DEVICE); 
	pcie_descr.offset=offset;

	// Initiator Reset
	write_reg( REG_DCSR, BIT_INITRESET);
	write_reg( REG_DCSR, BIT_INIT);

	// number of TLP 
	write_reg(REG_WDMATLPC, pcie_descr.tlp_count);
	// size of TLP
	write_reg(REG_WDMATLPS, pcie_descr.tlp_size);
	//pattern to be send <-------------TODO: remove in final version
	write_reg(REG_WDMATLPP, pcie_descr.pattern);

	// recieving buffer address
	write_reg(REG_WDMATLPA, pcie_descr.buffer_hw_addr+ (!pcie_descr.even_buffer?BUF_SIZE:0)+offset);

	//Make sure everything is set
	wmb();

	//Start DMA Transfer
	PRINT1(KERN_DEBUG DEV_N "DMA READY, starting\n");
	if(!send_interrupt){
		write_reg(REG_DDMACR,BIT_WDMASTART|BIT_WDMAINTDONE);
		wmb();
	}
	else{
		write_reg(REG_DDMACR,BIT_WDMASTART);
		wmb();
	}


};


/*
 * fpga_char_ioctl - ioctl handler
 * @file - unnused
 * @cmd - the ioctl call number defined in pcie_ioctl.h
 * @arg - argument of the ioctl call
 *
 * Checks which command has been called and executes it. Check for more 
 * information in pcie_ioctl.h
 */

long fpga_char_ioctl(struct file * file, unsigned int cmd, unsigned long arg){
	int ret= SUCCESS;	
	int retarg=0;
	int i;
	dma_addr_t offset;
	dma_addr_t transfer_size;
	offset=0;
	switch(cmd){
		case RESET_CARD:
			PRINT1(KERN_INFO DEV_N"IOCTL RESET_CARD\n");
			fpga_init_device();
			break;
		case START_STREAM:
			PRINT1(KERN_INFO DEV_N"IOCTL START_STREAM\n");
			pcie_descr.even_buffer=true;

			transfer_size=4*pcie_descr.tlp_size*pcie_descr.tlp_count;

			pcie_descr.wake=false;
			pcie_descr.pattern=0x2a2a2a2a;
			//last iteration done outside of the loop
			for(i=0;i<pcie_descr.dma_count-1;i++){
				run_dma(false, offset);		
				offset+=transfer_size;
			}
			pcie_descr.dma_done=false;
			pcie_descr.wake=true;
			run_dma(true,offset) ;		

			break;
		case NEXT_STREAM:
			PRINT1(KERN_INFO DEV_N"IOCTL NEXT_STREAM\n");
	
			if(!pcie_descr.dma_done){
				i=wait_event_interruptible_timeout(pcie_descr.irq_wait, pcie_descr.dma_done, msecs_to_jiffies(10));
				if(i==0){
					printk(KERN_ERR DEV_N"Transfer timeout\n");
					ret=-EIO;
					break;			
				}
			}
	
			if(pcie_descr.even_buffer){
				pcie_descr.even_buffer=false;
				retarg=0;
				pcie_descr.pattern=0x2b2b2b2b;
			}else{
				pcie_descr.even_buffer=true;
				retarg=BUF_SIZE;
				pcie_descr.pattern=0x2c2c2c2c;
			}


			pcie_descr.wake=false;

			transfer_size=4*pcie_descr.tlp_size*pcie_descr.tlp_count;
			//last iteration done outside of the loop
			for(i=0;i<pcie_descr.dma_count-1;i++){
				run_dma(false, offset);		
				offset+=transfer_size;
			}

			pcie_descr.dma_done=false;
			pcie_descr.wake=true;
			run_dma(true,offset) ;		

			if (copy_to_user((int *)arg,&retarg, sizeof(int))) {
				ret = -EFAULT;
				break;
			}

			break;
		case END_STREAM:
			PRINT1(KERN_INFO DEV_N"IOCTL END_STREAM\n");

			if(!pcie_descr.dma_done){
				i=wait_event_interruptible_timeout(pcie_descr.irq_wait, pcie_descr.dma_done, msecs_to_jiffies(10));
				if(i==0){printk(KERN_ERR DEV_N"timeout\n");
					ret=-EIO;
					break;			
				}
			}

			if(!pcie_descr.dma_done)
				wait_event_interruptible(pcie_descr.irq_wait, pcie_descr.dma_done);
			if(pcie_descr.even_buffer){
				pcie_descr.even_buffer=false;
				retarg= 0;
			}else{

				pcie_descr.even_buffer=true;
				retarg= BUF_SIZE;
			}
			if (copy_to_user((int *)arg,&retarg, sizeof(int))) {
				ret = -EFAULT;
				break;
			}
			break;
		case SET_DMACOUNT:
			PRINT2(KERN_INFO DEV_N"IOCTL SET_DMACOUNT:%d\n", (int)arg);
			pcie_descr.dma_count = (int) arg;
			break;
		case SET_TLPSIZE:
			if( (*(int *)arg) > 32 ) 
				goto err_tlp;
			pcie_descr.tlp_size  = *((int *) arg);
			break;
		case SET_TLPCOUNT:
			if( (*(int *)arg) > 128 ) 
				goto err_tlp;
			pcie_descr.tlp_count =  *((int *) arg);
		case MEMSET_WRITEBUFFER:
			PRINT3(KERN_DEBUG DEV_N "IOCTL MEMSET: %c(%x)\n",(char)arg,(int)arg);
			memset(pcie_descr.buffer,arg,BUF_SIZE*2);
			break;
		default:
			printk(KERN_ERR DEV_N "IOCTL ERR\n");
			return -ENOTTY;
	}

	return ret;
err_tlp:
	printk(KERN_ERR DEV_N "ioctl error: the XAPP1052 allows only a maximum %s", \
			"of 128 TLP's of 32 DW size\n");
	return -EINVAL;

}


/*
 * fpqa_irq_handler - interrupt handler
 * @irq - the raised interrupt number
 * @dev_id - device id
 *
 * Wake up the program.
 */
irqreturn_t fpga_irq_handler(int irq, void *dev_id)
{
	pcie_descr.dma_done = true;

	pci_unmap_single(gDev, pcie_descr.dma_addr+(! pcie_descr.even_buffer?BUF_SIZE:0)+pcie_descr.offset, BUF_SIZE, DMA_FROM_DEVICE); 
	PRINT1(KERN_DEBUG DEV_N "Interrupt Handler End ..\n");
	if(pcie_descr.wake){
		wake_up_interruptible(&pcie_descr.irq_wait);
	}
	return IRQ_HANDLED;
}

/*
 * fpga_char_read
 * @filp:	unnused
 * @buffer:	address of the user buffer to send data to
 * @count:	how much data the user requested
 * @offest:	unnused
 *
 * Reset the initiator, feed data into the device, start the dma
 * initiator and put asleep the program. After handling fpga_irq_handler 
 * the program will be waken up and the data placed in the right buffer.
 */
static ssize_t fpga_char_read(struct file *filp,	
		char *buffer,
		size_t count,	
		loff_t * offset)
{

	int tlp_size;
	int tlp_count;

	tlp_size = 32;
	tlp_count= 128;

	// Initiator Reset
	write_reg( REG_DCSR, BIT_INITRESET);
	write_reg( REG_DCSR, BIT_INIT);

	//number of TLP
	write_reg(REG_WDMATLPC, pcie_descr.tlp_count);
	//size of TLP
	write_reg(REG_WDMATLPS, pcie_descr.tlp_size);
	//pattern to be send <---------------TODO: remove in final version
	write_reg(REG_WDMATLPP, 0x2a2b2c2d);

	//recieving buffer address
	write_reg(REG_WDMATLPA, pcie_descr.buffer_hw_addr);

	//Make sure everything is set
	wmb();

	//Start DMA Transfer
	PRINT1(KERN_DEBUG DEV_N "DMA start\n");
	write_reg(REG_DDMACR,BIT_WDMASTART);

	pcie_descr.dma_done=false;
	wait_event_interruptible(pcie_descr.irq_wait, pcie_descr.dma_done );

	PRINT1(KERN_DEBUG DEV_N "Process waken\n");
	memcpy(buffer,(char*)pcie_descr.buffer,tlp_size*tlp_count*sizeof(char));

	return tlp_count*tlp_size*sizeof(char);
}

/*
 * fpga_char_write 
 * @filp:	unnused
 * @buffer:	address of the user buffer to send data from
 * @count:	how much data the user sends
 * @offest:	unnused
 *
 *
 *
 */
ssize_t
fpga_char_write(
		struct file *file,
		const char *buffer,
		size_t len, 
		loff_t * off)
{
	int tlp_size;
	int tlp_count;

	tlp_size = 32;
	tlp_count= 128;

	memcpy((char*)pcie_descr.buffer, buffer,tlp_size*tlp_count*sizeof(char));

	// Initiator Reset
	write_reg( REG_DCSR, BIT_INITRESET);
	write_reg( REG_DCSR, BIT_INIT);

	// number of TLP
	write_reg(REG_RDMATLPC, pcie_descr.tlp_count);
	// size of TLP
	write_reg(REG_RDMATLPS, pcie_descr.tlp_size);
	// expected pattern <-------TODO: remove in finale version 
	write_reg(REG_RDMATLPP, 0x2a2b2c2d);

	// address of sending buffer
	write_reg(REG_RDMATLPA, pcie_descr.buffer_hw_addr);

	//Make sure everything is set
	wmb();

	//Start DMA Transfer
	PRINT1(KERN_DEBUG DEV_N "DMA start\n");
	write_reg(REG_DDMACR,BIT_RDMASTART);

	pcie_descr.dma_done=false;
	wait_event_interruptible(pcie_descr.irq_wait, pcie_descr.dma_done );

	PRINT1(KERN_DEBUG DEV_N "Process waken\n");


	return tlp_count*tlp_size*sizeof(char);
}

/*
 * fpga_init_device - feed starting data into the device
 */
void fpga_init_device(void)
{
	//Reset the initiator
	write_reg( REG_DCSR, BIT_INITRESET);
	write_reg( REG_DCSR, BIT_INIT);
	write_reg( REG_DDMACR, 0);				//make sure DMACR is off
	write_reg( REG_WDMATLPA, pcie_descr.buffer_hw_addr);	//WRITE DMA TLP Address
	write_reg( REG_WDMATLPS, 0x32);				//		Size
	write_reg( REG_WDMATLPC, 0x128);			//		Count
	write_reg( REG_WDMATLPP, 0x1e2e3e4e);			//		Pattern
	write_reg( REG_RDMATLPP, 0xfeedbeef);			//READ  DMA TLP Pattern
	write_reg( REG_RDMATLPA, pcie_descr.buffer_hw_addr);	//		Address
	write_reg( REG_RDMATLPS, 0x20);				//		Size
	write_reg( REG_RDMATLPC, 0x2000);			//		Count
}

/*
 * fpga_probe - device inserted
 * @dev: 		device description structure
 * @pcie_device_id: 	unnused
 *
 * Enable the device and set as bus master, request region and remap it,
 * prepare the ISR and DMA, alloc memory for read/write buffer(ping-pong style)
 * and register device in udev system.
 */
int fpga_probe (struct pci_dev *dev, const struct pci_device_id *id)   /* New device inserted */
{
	int result;

	//prepare the global variable
	gDev=dev;

	result = pci_enable_device (dev);
	if(result){
		printk(KERN_ERR DEV_N "Couldn't enable the device\n");
		goto err_enable_device;
	}
	pci_set_master(dev);

	pcie_descr.base_start = pci_resource_start(dev, 0);
	pcie_descr.base_end = pci_resource_end(dev, 0);
	pcie_descr.base_len = pcie_descr.base_end-pcie_descr.base_start+1;

	PRINT3(KERN_DEBUG DEV_N "base_start=0x%08lx;  base_len=0x%08lx\n",pcie_descr.base_start,pcie_descr.base_len);

	if(!pcie_descr.base_start || !pcie_descr.base_len){
		printk(KERN_ERR DEV_N "Base address error.\n");
		goto err_base_address;
	}

	result =pci_request_regions(dev,"fpga_pcie");
	//result =request_mem_region(pci_resource_start(dev,0),pci_resource_len(dev,0),"fpga_pcie");
	if (result){
		printk(KERN_ERR DEV_N "Couldn't grab region\n");
		goto err_region;
	}

	pcie_descr.region = ioremap(pcie_descr.base_start, pcie_descr.base_len);
	PRINT2(KERN_DEBUG DEV_N "region=0x%p;\n",pcie_descr.region);

	if (!pcie_descr.region){
		printk(KERN_ERR DEV_N "Coudln't map the region in the system.\n");
		goto err_ioremap;
	}

	// enable message signaled interrupts, returns 0 on success  
	result = pci_enable_msi(dev);
	if(result){     
		printk(KERN_ERR DEV_N  "\033[31mERR Could not enable MSI\033[0m, :%x", result);
		pci_disable_msi(dev);
	}


	pcie_descr.irq = dev->irq;	
	printk(KERN_DEBUG DEV_N "Device IRQ: %d\n", pcie_descr.irq);
	result =request_irq(dev->irq, &fpga_irq_handler, IRQF_SHARED | IRQF_SAMPLE_RANDOM, DEVICE_NAME, dev);
	if (0 > result ) {
		printk(KERN_ERR DEV_N "Probe: Unable to allocate IRQ: %x", result);
		goto err_irq_request;
	}

	init_waitqueue_head (&pcie_descr.irq_wait);

	result = pci_set_dma_mask(dev, DMA_BIT_MASK(64));
	if(result){
		printk(KERN_ERR DEV_N "Couldn't set DMA mask\n");
		goto err_dma_mask;
	}


	if (0> request_dma(1, "fpga_pcie")){
		printk(KERN_ERR DEV_N "Request dma failed\n");
		goto err_request_dma;
	}

	pcie_descr.buffer = pci_alloc_consistent(dev, 2* BUF_SIZE, &(pcie_descr.buffer_hw_addr));
	if (NULL == pcie_descr.buffer) {
		printk(KERN_ERR DEV_N "Unable to allocate Buffer.\n");
		goto err_buffer;
	}
	PRINT3(KERN_DEBUG DEV_N "Buffer Allocation: %X->%X\n", (unsigned int)pcie_descr.buffer, (unsigned int)pcie_descr.buffer_hw_addr);

	result= misc_register(&fpga_dev);
	if(result){
		printk(KERN_ERR DEV_N "Registering char fpga_char failed with %d\n", result);
		goto err_char_dev;
	}

	//reboot the device 
	fpga_init_device();

	printk(KERN_INFO "Successfully claimed device: %s\n", pci_name(dev));
	if (HAVE_UNLOCKED_IOCTL!=1){
		printk(KERN_CRIT "Kernel doesn't support ioctl\n");
	}
	return SUCCESS;

	//ERROR handling of fpga_probe
err_char_dev:
	pci_free_consistent(dev,BUF_SIZE,pcie_descr.buffer,pcie_descr.buffer_hw_addr);
err_buffer:
	free_dma(1);
err_request_dma:
err_dma_mask:
	free_irq(pcie_descr.irq, dev);
	pci_disable_msi(dev);
err_irq_request:
	if(pcie_descr.region!=NULL)
		iounmap(pcie_descr.region);
err_ioremap:
	pci_release_region(dev,0);
err_region:
err_base_address:
	pci_disable_device(dev);
err_enable_device:
	return -1;
}

/*
 * fpga_remove - device removed from system
 * @pdev: device descriptor
 *
 * Undo all prepared in fpga_probe
 */
	void fpga_remove (struct pci_dev *pdev){
		if(pdev)
		{
			pci_free_consistent(pdev,BUF_SIZE,pcie_descr.buffer,pcie_descr.buffer_hw_addr);

			free_irq(pdev->irq, pdev);
			pci_disable_msi(pdev);

			free_dma(1);
			if(pcie_descr.region!=NULL)
				iounmap(pcie_descr.region);

			pci_release_region(pdev,0);

			pci_disable_device(pdev);
			misc_deregister(&fpga_dev);

			printk(KERN_INFO "Succesfully removed: %s\n", pci_name(pdev));
		}

	}

/*
 * fpga_init - driver inserted
 */
static int __init fpga_init(void){
	int result;

	result=pci_register_driver(&fpga_pcie_driver);
	if(result){
		printk(KERN_ERR DEV_N "Couldn't register the driver\n");
		return result;
	}

	pcie_descr.tlp_size=32;
	pcie_descr.tlp_count=128;
	pcie_descr.dma_count=1;
	pcie_descr.pattern=0x2a2a2a2a;


	PRINT1(KERN_DEBUG DEV_N "Module loaded.\n");

	return SUCCESS;
}

/*
 * fpga_exit - driver removed
 */
static void __exit fpga_exit(void){
	pci_unregister_driver(&fpga_pcie_driver);
	PRINT1(KERN_DEBUG DEV_N "Module Unloaded.\n");
}

module_init(fpga_init);
module_exit(fpga_exit);

