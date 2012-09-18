
#include <vector>
#include "DMABuffer.h"
#include "DMAEngine.h"
#include "DMAEngineWG.h"
#include "DMADescriptor.h"
#include "DMADescriptorWG.h"
#include "DMADescriptorListWG.h"
#include "Driver.h"
#include "Exception.h"
#include "PCIDriver.h"
#include "pciDriver/lib/pciDriver.h"
#include <iostream>
#include "util/Timer.h"

/* Call usleep() with 250 between checking for status changes. */
#define TIMEOUT_PRECISION_USLEEP 1

using namespace mprace;
using namespace std;

const unsigned int DMAEngineWG::STATUS_TOUT = 0x00000010;
const unsigned int DMAEngineWG::STATUS_BUSY = 0x00000002;
const unsigned int DMAEngineWG::STATUS_DONE = 0x00000001;
const unsigned int DMAEngineWG::CTRL_RESET = 0x0000000A;
const unsigned int DMAEngineWG::CTRL_END   = 0x00000100;
const unsigned int DMAEngineWG::CTRL_INC   = 0x00008000;
const unsigned int DMAEngineWG::CTRL_UPA   = 0x00100000;
const unsigned int DMAEngineWG::CTRL_LAST  = 0x01000000;
const unsigned int DMAEngineWG::CTRL_V     = 0x02000000;
const unsigned int DMAEngineWG::CTRL_EDI   = 0x10000000;
const unsigned int DMAEngineWG::CTRL_EEI   = 0x20000000;
const unsigned int DMAEngineWG::CTRL_ESEI  = 0x40000000;
//const unsigned int DMAEngineWG::CTRL_BAR   = 0x00010000;	// new bitfiles!

const unsigned int DMAEngineWG::INTE_CH0   = 0x00000002;
const unsigned int DMAEngineWG::INTE_CH1   = 0x00000001;

const unsigned int DMAEngineWG::IRQ_SRC_CH0   = 0;
const unsigned int DMAEngineWG::IRQ_SRC_CH1   = 1;

DMAEngineWG::DMAEngineWG( Driver& drv, unsigned int *base0, unsigned int *base1, unsigned int *INTE, unsigned int *INTS, unsigned int *DMATRANS0, unsigned int *DMATRANS1 )
	: DMAEngine(drv)
{
	// Setup base channel pointers
	channel[0] = base0;
	channel[1] = base1;

	// Setup Interrupt Register pointers
	inte = INTE;
	ints = INTS;

        // Setup DMA Actual Transferred Register pointer
        dmatrans[0] = DMATRANS0;
        dmatrans[1] = DMATRANS1;

	// Reset both DMA channels
	reset(0);
	reset(1);

	// Default value for use Interrupts
	useInterrupts = false;

	// Default value for loop limit
	loop_limit = 10;

	// Calibrated timer needed for the timeout functionality
	if (!mprace::util::Timer::is_calibrated())
		mprace::util::Timer::calibrate();
}

DMAEngineWG::~DMAEngineWG()
{
}

void DMAEngineWG::reset(const unsigned int ch)
{
	(channel[ch])[7] = CTRL_RESET | CTRL_V;
}

void DMAEngineWG::write(const unsigned int ch, const DMADescriptorWG& d)
{
	DMADescriptorWG::descriptor *desc = d.native;

	(channel[ch])[0] = desc->per_addr_h;
	(channel[ch])[1] = desc->per_addr_l;
	(channel[ch])[2] = desc->host_addr_h;
	(channel[ch])[3] = desc->host_addr_l;
	(channel[ch])[4] = desc->next_bda_h;
	(channel[ch])[5] = desc->next_bda_l;
	(channel[ch])[6] = desc->length;
	(channel[ch])[7] = desc->control;		// control is written at the end, starts DMA
}

DMAEngine::DMAStatus DMAEngineWG::getStatus(const unsigned int ch)
{
	// Read the status register of the channel
	unsigned long status = (channel[ch])[8];

	if ((status & STATUS_BUSY) && (status & STATUS_DONE))
		return ERROR;
	else if ( status & STATUS_BUSY )
		return BUSY;
	else if ( status & STATUS_TOUT )
		return TIMEOUT;
	else
		return IDLE;
}

void DMAEngineWG::enableInterrupt(const unsigned int ch)
{
	unsigned int mask;

	switch (ch) {
		case 0:
			mask = INTE_CH0;
			break;
		case 1:
			mask = INTE_CH1;
			break;
	}

	*inte |= mask;
}

void DMAEngineWG::disableInterrupt(const unsigned int ch)
{
	unsigned int mask;

	switch (ch) {
		case 0:
			mask = !INTE_CH0;
			break;
		case 1:
			mask = !INTE_CH1;
			break;
	}

	*inte &= mask;
}

void DMAEngineWG::waitForInterrupt(const unsigned int ch)
{
	unsigned int src;

	switch (ch) {
	case 0:
		src = IRQ_SRC_CH0;
		break;
	case 1:
		src = IRQ_SRC_CH1;
		break;
	}

	drv->waitForInterrupt(src);
}

void DMAEngineWG::waitChannel(const unsigned int ch, const float timeout)
{
	if (useInterrupts) {
                mprace::util::Timer timer;

                timer.start();

		DMAStatus dma_status;
		if (((dma_status = getStatus(ch)) != IDLE) && (dma_status != TIMEOUT)){
			enableInterrupt(ch);
			waitForInterrupt(ch);
			disableInterrupt(ch);
		}

                timer.stop();

                if (timeout > 0.0 && timer.asMillis() > timeout) {
                        if (saved[ch]) {
                                restoreSavedData(ch);
                                saved[ch] = false;
                        }
                        if (dmatrans[ch] != NULL)
                                throw new mprace::Exception(Exception::DMA_TIMEOUT, *dmatrans[ch]);
                        else throw new mprace::Exception(Exception::DMA_TIMEOUT);
                }
	}
	else {
		unsigned long status = (channel[ch])[8];
                unsigned int loop_count = 0;
		DMAStatus dma_status;
		mprace::util::Timer timer;

		timer.start();

		/* Wait for the card being IDLE (success) or the DMA timeout (failure) */
		while (((dma_status = getStatus(ch)) != IDLE) && (dma_status != TIMEOUT)) {
			timer.stop();
			if (timeout > 0.0 && timer.asMillis() > timeout) {
				if (saved[ch]) {
					restoreSavedData(ch);
					saved[ch] = false;
				}

				if (dmatrans[ch] != NULL)
					throw new mprace::Exception(Exception::DMA_TIMEOUT, *dmatrans[ch]);
				else
					throw new mprace::Exception(Exception::DMA_TIMEOUT);
			}

			/* Before calling usleep(), we try 10 times if the DMA finishes.
			 * If it did not finish, it is likely that it will timeout and
			 * we call usleep() to prevent hogging the CPU. The usleep() will
			 * make the scheduler select other processes and wait quite
			 * long (~ 15 msec) until our process gets cpu time again. This
			 * would drain the performance to < 1 MB/s. */
			if (loop_count++ > getLoopLimit())
				usleep(TIMEOUT_PRECISION_USLEEP);

			status = (channel[ch])[8];
		}
//		cerr << "Timeout after loop? " << (getStatus(ch) == TIMEOUT ? "yes" : "no") << endl;
//		cerr << "idle after loop? " << (getStatus(ch) == IDLE ? "yes" : "no") << endl;
	}

	// If a descriptor was saved, restore it to it correct position in the SG list.
	// This is only for UserMemory buffers
	if (saved[ch]) {
		restoreSavedData(ch);
		saved[ch] = false;
	}
}

void DMAEngineWG::restoreSavedData(unsigned int ch)
{
	DMADescriptorList *dlist = const_cast<DMADescriptorList *>(saved_data[ch].saved_buf->descriptors);
	DMADescriptorListWG& list = static_cast<DMADescriptorListWG&>(*dlist);

	unsigned int max_descriptor = list.getSize()-1;
	unsigned int index;

	if (saved_data[ch].saved_offset != 0) {
		// Restore initial descriptor
		index = saved_data[ch].saved_init_descriptor;
		list[index].setHostAddress( list[index].getHostAddress() - saved_data[ch].saved_offset );
		list[index].setLength( list[index].getLength() + saved_data[ch].saved_offset );
	}

	if ( saved_data[ch].saved_init_descriptor != max_descriptor )
		list[ saved_data[ch].saved_init_descriptor+1 ].setControl( 0 );

	index = saved_data[ch].saved_last_descriptor;
	list[index].setLength( saved_data[ch].saved_length );
	list[index].setControl( 0 );
	if (saved_data[ch].saved_last_descriptor == max_descriptor) {
		list[index].setNextDescriptorAddress( 0UL );
	}
	else {
		list[index].setNextDescriptorAddress( list[index+1].getPhysicalAddress() );
	}
}

void DMAEngineWG::fillDescriptorList(DMABuffer& buf)
{
	if (buf.getType() == DMABuffer::KERNEL) {
		// No descriptor list is needed for a Kernel buffer
		buf.descriptors = NULL;
	}

	if (buf.getType() == DMABuffer::KERNEL_PIECES) {
		DMADescriptorListWG *dlist = new DMADescriptorListWG(buf, DMADescriptorListWG::USER);
		buf.descriptors = dlist;
		DMADescriptorListWG& list = *dlist;

		unsigned long long base_ha = buf.kBuf->getPhysicalAddress();
		unsigned int step = buf.size() / buf.kernel_pieces;
		unsigned int count = 0;

		for(int i=0 ; i<buf.kernel_pieces ; i++) {
			DMADescriptorWG *descriptor;
			descriptor = &(list[i]);

			descriptor->setHostAddress( base_ha + i*step );
			descriptor->setLength( (i == (buf.kernel_pieces-1)) ? buf.size()-count : step );
			descriptor->setControl( 0 );
			descriptor->setPeripheralAddress(0UL);
			descriptor->setNextDescriptorAddress( (i == (buf.kernel_pieces-1)) ? 0UL : list[i+1].getPhysicalAddress() );
			count += step;
		}
	}


	if (buf.getType() == DMABuffer::USER) {
		DMADescriptorListWG *dlist = new DMADescriptorListWG(buf, DMADescriptorListWG::USER);
		buf.descriptors = dlist;
		DMADescriptorListWG& list = *dlist;

		// Set the pointer to the next descriptor
		for( int i=0 ; i < buf.uBuf->getSGcount() ; i++ ) {
			DMADescriptorWG *descriptor;
			descriptor = &list[i];

			descriptor->setHostAddress( buf.uBuf->getSGentryAddress(i) );
			descriptor->setLength( buf.uBuf->getSGentrySize(i) );
			descriptor->setControl( 0 );
			descriptor->setPeripheralAddress(0UL);
			descriptor->setNextDescriptorAddress( (i == (buf.uBuf->getSGcount()-1)) ? 0UL : list[i+1].getPhysicalAddress() );
		}
	}
}

void DMAEngineWG::releaseDescriptorList(DMABuffer& buf)
{
	// Release the Kernel Buffer associated with the Descriptor List of the buffer
	if ((buf.getType() == DMABuffer::USER) || (buf.getType() == DMABuffer::KERNEL_PIECES)) {
		delete buf.descriptors;
	}
}

void DMAEngineWG::host2board(const unsigned int bar, const unsigned int addr,
		const DMABuffer& buf, const unsigned int count, const unsigned
		int offset, const bool inc, const bool lock, const float timeout)
{
        /* Checks if count != 0 */
        if (count == 0)
                throw Exception(Exception::EMPTY_TRANSFER);

        /* Checks if the transfer-size exceeds the buffer size */
        if (buf.size() < (offset + count) * sizeof(int))
                throw Exception(Exception::ADDRESS_OUT_OF_RANGE);

	this->reset(0);

#ifndef OLD_REGISTERS
	if (!useInterrupts)
		this->disableInterrupt(0);
#endif

	if (buf.getType() == DMABuffer::KERNEL) {
		// Single descriptor, easy
		pciDriver::KernelMemory *kb = buf.kBuf;
		DMADescriptorWG d;
		unsigned long control;
		unsigned int CTRL_BAR = (bar & 0x00000007) << 16;

		d.setHostAddress( kb->getPhysicalAddress() + offset );
		d.setPeripheralAddress(addr*4);
		d.setNextDescriptorAddress(0L);
		d.setLength(count*4);
		control = CTRL_LAST | CTRL_BAR;
#ifndef OLD_REGISTERS
		control |= CTRL_UPA | CTRL_V;
		control |= (useInterrupts) ? CTRL_EDI : 0x0;
#endif
		control |= (inc) ? CTRL_INC : 0x0;
		d.setControl(control);

		saved[0] = false;

		buf.sync(DMABuffer::BOTH);

		// host2board is channel 0
		this->write(0,d);
	}

	if ((buf.getType() == DMABuffer::USER) || (buf.getType() == DMABuffer::KERNEL_PIECES)) {
		buf.sync(DMABuffer::BOTH);
		sendDescriptorList(bar,addr,buf,count,offset,inc,0);
	}

	if (lock)
		this->waitChannel(0, timeout);
}

void DMAEngineWG::board2host(const unsigned int bar, const unsigned int addr,
		DMABuffer& buf, const unsigned int count, const unsigned int
		offset, const bool inc, const bool lock, const float timeout)
{
        /* Checks if count != 0 */
        if (count == 0)
                throw Exception(Exception::EMPTY_TRANSFER);

        /* Checks if the transfer-size exceeds the buffer size */
        if (buf.size() < (offset + count) * sizeof(int))
                throw Exception(Exception::ADDRESS_OUT_OF_RANGE);

	this->reset(1);

#ifndef OLD_REGISTERS
	if (!useInterrupts)
		this->disableInterrupt(1);
#endif

        if (offset > 0 || count < buf.size() / 4) {
		/* If the transfersize is not equal to the buffersize we need
		 * to sync the buffer into the SWIOTLB, otherwise the SWIOTLB
		 * might sync invalid data back into the buffer (it will sync
		 * more than actually transferred because only the device
		 * itself knows where it will transfer data to).  */
                buf.sync(DMABuffer::TODEVICE);
        }

	if (buf.getType() == DMABuffer::KERNEL) {
		// Single descriptor, easy
		pciDriver::KernelMemory *kb = buf.kBuf;
		DMADescriptorWG d;
		unsigned long control;
		unsigned int CTRL_BAR = (bar & 0x00000007) << 16;

		d.setPeripheralAddress(addr*4);
		d.setHostAddress( kb->getPhysicalAddress() + offset );
		d.setNextDescriptorAddress(0);
		d.setLength(count*4);
		control = CTRL_LAST | CTRL_BAR;
#ifndef OLD_REGISTERS
		control |= CTRL_UPA | CTRL_V;
		control |= (useInterrupts) ? CTRL_EDI : 0x0;
#endif
		control |= (inc) ? CTRL_INC : 0x0;
		d.setControl(control);

		saved[1] = false;

		// board2host is channel 1
		this->write(1,d);
	}

	if ((buf.getType() == DMABuffer::USER) || (buf.getType() == DMABuffer::KERNEL_PIECES)) {
		sendDescriptorList(bar,addr,buf,count,offset,inc,1);
	}

	if (lock) {
                try {
                        this->waitChannel(1, timeout);
                } catch (...) {
                        buf.sync(DMABuffer::FROMDEVICE);
                        throw;
                }
		buf.sync(DMABuffer::FROMDEVICE);
	}
}

void DMAEngineWG::sendDescriptorList(const unsigned int bar, const unsigned int addr, const DMABuffer& buf, const unsigned int count, const unsigned int offset, const bool inc, const unsigned int ch)
{
	pciDriver::UserMemory *uBuf = buf.uBuf;

	// get the list of descriptors for this buffer
	DMADescriptorList *dlist = buf.descriptors;
	DMADescriptorListWG& list = *(static_cast<DMADescriptorListWG*>(dlist));

	unsigned int nr_descriptors = list.getSize();

	unsigned int init_descriptor,last_descriptor;
	unsigned int control_word;
	unsigned int CTRL_BAR = (bar & 0x00000007) << 16;
	DMADescriptorWG temp;

	control_word = CTRL_V | CTRL_BAR;
	control_word |= (inc) ? CTRL_INC : 0x0;
	#ifdef OLD_REGISTERS
		control_word |= (useInterrupts) ? CTRL_EDI : 0x0;
	#endif

	// Check first for the fast and easy conditions
	// Optimize for the fast case
	if ((offset==0) && ((count*4)==buf.size())) {
		// set first, second and last descriptors
		last_descriptor = nr_descriptors-1;

                /* because the board will propagate the control_word of the
                 * first descriptor to all other descriptors, we need to set
                 * the "real" control_word on the second descriptor (as the
                 * first one and the last one have special flags). */
		if (nr_descriptors > 2)
			list[1].setControl( control_word );

		list[last_descriptor].setControl( control_word | CTRL_LAST );

		saved_data[ch].saved_last_descriptor = last_descriptor;
		saved_data[ch].saved_length = list[last_descriptor].getLength();
		saved_data[ch].saved_offset = 0;
		saved_data[ch].saved_init_descriptor = 0;
		saved_data[ch].saved_buf = &buf;
		saved[ch] = true;

		// write first descriptor
		temp = list[0];
		temp.setPeripheralAddress( addr*4 );
		temp.setControl( (control_word | CTRL_UPA) | ((nr_descriptors == 1) ? CTRL_LAST : 0x0) );

		//list.print();
		list.sync();

		// Send
		this->write(ch,temp);
	} // end fast condition: offset=0, length=max
	else {
		// We need either to apply an offset, end the buffer before its limit, or both.
		// get to the starting point
		unsigned int init_descriptor = 0;
		unsigned int byte_count=0;
		unsigned int byte_offset = offset*4;
		unsigned int block_offset=0;

		if (offset != 0) {
			while (init_descriptor < nr_descriptors) {
				if (byte_offset > byte_count + list[init_descriptor].getLength()) {
					byte_count += list[init_descriptor].getLength();
					init_descriptor++;
				} else {
                                        /* We found the descriptor with which we will start.
                                         * Because we may have skipped some descriptors, we
                                         * need to decrease the offset by these skipped bytes */
					block_offset = byte_offset - byte_count;
					break;
				}
			}
		}

                last_descriptor = init_descriptor;

		byte_count = (count * 4);

		while (last_descriptor < nr_descriptors) {
                        bool fits_in_descriptor;

                        /* Check if the amount of bytes we want to transfer fits into
                         * this descriptor. For the first descriptor, we need to take
                         * the block offset into account (the descriptor length is not
                         * yet modified as we might need to save it when there is only
                         * one descriptor). */
                        if (last_descriptor == init_descriptor)
                                fits_in_descriptor = (byte_count <= (list[init_descriptor].getLength() - block_offset));
                        else fits_in_descriptor = (byte_count <= list[last_descriptor].getLength());

			if (fits_in_descriptor) {
				// save the current descriptor
				// we are interested only in the descriptor number and the length.
				saved_data[ch].saved_length = list[last_descriptor].getLength();
				saved_data[ch].saved_offset = block_offset;
				saved_data[ch].saved_init_descriptor = init_descriptor;
				saved_data[ch].saved_last_descriptor = last_descriptor;
				saved_data[ch].saved_buf = &buf;
				saved[ch] = true;

				// set this as truly the last descriptor
				list[last_descriptor].setLength( byte_count );
				list[last_descriptor].setNextDescriptorAddress( 0L );
				list[last_descriptor].setControl( control_word | CTRL_LAST );
				break;
			} else {
                                if (last_descriptor == init_descriptor)
                                        byte_count -= (list[init_descriptor].getLength() - block_offset);
                                else byte_count -= list[last_descriptor].getLength();
				last_descriptor++;
			}
		}

		// Adjust control word of second descriptor
		if (init_descriptor+1 < last_descriptor)
			list[ init_descriptor+1 ].setControl( control_word );

		temp = list[init_descriptor];
                unsigned long long new_host_address = temp.getHostAddress() + block_offset;

                temp.setHostAddress(new_host_address);
                list[init_descriptor].setHostAddress(new_host_address);

                /* If only one descriptor is involved, we have already set the
                 * correct length (byte_count) a few lines above, so do not
                 * touch it here. */
                if (init_descriptor != last_descriptor) {
                        unsigned long new_length = temp.getLength() - block_offset;
                        temp.setLength(new_length);
                        list[init_descriptor].setLength(new_length);
                }

		temp.setPeripheralAddress( addr*4 );
		temp.setControl( (control_word | CTRL_UPA) | ((init_descriptor == last_descriptor) ? CTRL_LAST : 0x0) );

		//list.print();
		list.sync();

		// Send
		this->write(ch,temp);
	} // End conditional cases
}
