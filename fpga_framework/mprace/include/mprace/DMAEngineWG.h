#ifndef DMAENGINEWG_H_
#define DMAENGINEWG_H_

#include "DMAEngine.h"

namespace mprace {

class Driver;
class DMADescriptorWG;

// This is needed to compile properly in x86_64
typedef unsigned int * puint;

/**
 * DMA Engine interface for the Wengxue Gao's (WG) DMA controller.
 *
 * @author  Guillermo Marcus
 * @version $Revision: 1.12 $
 * @date    $Date: 2009-05-29 13:48:46 $
 */
class DMAEngineWG : public DMAEngine {
public:
	/**
	 * Creates an instance of the DMA Engine, to interface with Wenxue's design.
	 * @param driver The Driver to use to communicate with the driver. Needed to create DMA buffers.
	 * @param base0 Pointer to the base address to access the Channel0 descriptor (DMA_DOWN).
	 * @param base1 Pointer to the base address to access the Channel1 descriptor (DMA_UP).
	 * @param inte Pointer to the Interrupt Enable Register (IER).
	 * @param ints Pointer to the Interrupt Status Register (ISR).
	 */
	DMAEngineWG( Driver& driver, unsigned int *base0, unsigned int *base1, unsigned int *inte, unsigned int *ints, unsigned int *dmatrans0, unsigned int *dmatrans1 );

	/**
	 * Delete the DMA Engine
	 */
	virtual ~DMAEngineWG();

	/**
	 * Get the current status of a DMA channel.
	 *
	 * @param channel The channel to query
	 */
	virtual inline DMAStatus getStatus(const unsigned int channel);

	/**
	 * Reset the status of a DMA channel.
	 *
	 * @param channel The channel to reset
	 */
	virtual void reset(const unsigned int channel);

	/**
	 * Performs a DMA transaction from host to board.
	 *
	 * @param bar    The BAR number in the board.
	 * @param addr   The address in the board (dword address).
	 * @param buf    The DMA buffer in the host.
	 * @param count  Number of bytes to transfer.
	 * @param offset Initial dword to transfer.
	 * @param inc    If the address in the board is incremented or not.
	 * @param lock   If the function waits until the transaction is complete or not.
	 */
	virtual void host2board(const unsigned int bar, const unsigned int addr,
			const DMABuffer& buf, const unsigned int count,
			const unsigned int offset=0, const bool inc=true,
			const bool lock=true, const float timeout = 0.0);

	/**
	 * Performs a DMA transaction from board to host.
	 *
	 * @param bar    The BAR number in the board.
	 * @param addr   The address in the board (dword address).
	 * @param buf    The DMA buffer in the host.
	 * @param count  Number of bytes to transfer.
	 * @param offset Initial dword to transfer.
	 * @param inc    If the address in the board is incremented or not.
	 * @param lock   If the function waits until the transaction is complete or not.
	 */
	virtual void board2host(const unsigned int bar, const unsigned int addr,
			DMABuffer& buf, const unsigned int count,
			const unsigned int offset=0, const bool inc=true,
			const bool lock=true, const float timeout = 0.0);

	/**
	 * Fills the Descriptor List of the the DMA Buffer for the current engine.
	 *
	 * @param buf The DMA Buffer.
	 */
	virtual void fillDescriptorList(DMABuffer& buf);

	/**
	 * Release the Descriptor List of the the DMA Buffer for the current engine.
	 *
	 * @param buf The DMA Buffer.
	 */
	virtual void releaseDescriptorList(DMABuffer& buf);

	/**
	 * Set the DMAEngine to use interrupts or not.
	 * This is a global setting, and should not be change while DMA transactions are ongoing.
	 * @param val True to use interrupts, false to poll the status.
	 */
	inline void setUseInterrupts(bool val) { useInterrupts = val; }

	/**
	 * Get the use interrupts value.
	 */
	inline bool getUseInterrupts() { return useInterrupts; }

	/**
	 * set the limit of wait_loop in waitChanne().
	 * this method is called by the constructor of the boards
	 * @param limit of the loop
	 */
	inline void setLoopLimit(unsigned int limit) { loop_limit = limit; }

	/**
	 * Get the loop limit
	 */
	inline unsigned int getLoopLimit() { return loop_limit; }

	/**
	 * Wait for the channel to be done.
	 * @param channel The channel number to wait for.
	 */
	void waitChannel(const unsigned int channel, const float timeout = 0.0);

protected:
	const static unsigned int STATUS_TOUT;		//** Status word, Timeout bit.
	const static unsigned int STATUS_BUSY;		//** Status word, Busy bit.
	const static unsigned int STATUS_DONE;		//** Status word, Done bit.
	const static unsigned int CTRL_RESET;		//** Control word, Reset command.
	const static unsigned int CTRL_END;			//** Control word, End bit.
	const static unsigned int CTRL_INC;			//** Control word, Inc bit.
	const static unsigned int CTRL_LAST;		//** Control word, Last bit.
	const static unsigned int CTRL_EDI;			//** Control word, EDI bit.
	const static unsigned int CTRL_EEI;			//** Control word, EEI bit.
	const static unsigned int CTRL_ESEI;		//** Control word, ESEI bit.
	const static unsigned int CTRL_UPA;			//** Control word, UPA bit.
	const static unsigned int CTRL_V;			//** Control word, V bit.
//	const static unsigned int CTRL_BAR;			//** Control word, BAR number.

	const static unsigned int INTE_CH0;			//** Interrupt Enable Register, Channel 0.
	const static unsigned int INTE_CH1;			//** Interrupt Enable Register, Channel 1.

	const static unsigned int IRQ_SRC_CH0;			//** Interrupt Source queue, Channel 0.
	const static unsigned int IRQ_SRC_CH1;			//** Interrupt Source queue, Channel 1.

	/**
	 * Write a DMA Descriptor to a channel control address.
	 * @param channel The channel number to write the descriptor to.
	 * @param desc DMADescriptor to write.
	 */
	inline void write(const unsigned int channel, const DMADescriptorWG& desc);

	/**
	 * Enable Interrupts for a DMA channel.
	 *
	 * @param channel The channel to modify
	 */
	inline void enableInterrupt(const unsigned int channel);

	/**
	 * Disable Interrupts for a DMA channel.
	 *
	 * @param channel The channel to modify
	 */
	inline void disableInterrupt(const unsigned int channel);

	/**
	 * Wait the Interrupt from a given DMA channel.
	 *
	 * @param channel The channel to modify
	 */
	inline void waitForInterrupt(const unsigned int ch);

	/**
	 * Prepares and sends a descriptor list for a DMA transaction
	 *
	 * @param bar    The BAR number to use in the CTRL word.
	 * @param addr   The address in the board (dword address).
	 * @param buf    The DMA buffer in the host.
	 * @param count  Number of bytes to transfer.
	 * @param offset Initial dword to transfer.
	 * @param inc    If the address in the board is incremented or not.
	 * @param ch     Channel to perform the transaction
	 */
	void sendDescriptorList(const unsigned int bar, const unsigned int addr, const DMABuffer& buf, const unsigned int count, const unsigned int offset, const bool inc, const unsigned int ch);

	/**
	 * Restore data saved for a descriptor list
	 *
	 * @param ch     Channel that saved the data
	 */
	void restoreSavedData(unsigned int ch);

private:
	volatile puint channel[2];	//** pointers to each channel base address
	volatile puint inte;		//** pointer to the Interrupt Enable Register.
	volatile puint ints;		//** pointer to the Interupt Status Register.
        volatile puint dmatrans[2];     //** pointer to the DMA Actual Transferred Registers
	bool useInterrupts;			//** Use interrupts on the DMA transfer / wait.
	unsigned int loop_limit;	//** limit for the loop in waitChannel

	typedef struct {
		DMABuffer const *saved_buf;			//** The buffer related to the saved data.
		unsigned int saved_init_descriptor;	//** Saved descriptor index, when transferring an user buffer.
		unsigned int saved_last_descriptor;	//** Saved descriptor index, when transferring an user buffer.
		unsigned int saved_offset;		//** Saved descriptor offset (at init), when transferring an user buffer.
		unsigned int saved_length;		//** Saved descriptor length (at last), when transferring an user buffer.
	} saved_data_t;

	bool saved[2];						//** Signal that valid data was saved for a channel.
	saved_data_t saved_data[2];			//** Data saved for a SG transaction in a channel.
}; /* class DMAEngineWG */

} /* namespace mprace */

#endif /*DMAENGINEWG_H_*/
