#ifndef INTERRUPTGENERATOR_H_
#define INTERRUPTGENERATOR_H_

namespace mprace {

// This is needed to compile properly in x86_64
typedef unsigned int * puint;

/**
 * Class controller the functionality of the Interrupt Generator Test module.
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.2 $
 * @date    $Date: 2007-11-04 20:58:57 $
 */
class InterruptGenerator {
public:
	InterruptGenerator(unsigned int *base, unsigned int *inte, unsigned int *ints );
	
	~InterruptGenerator();
	
	inline unsigned int getAssertCount() { return *igan; }
	
	inline unsigned int getDeassertCount() { return *igdn; }
	
	inline void reset() { *igc = CTRL_RESET; }
	
	inline void setLatency(unsigned int nanoseconds) { *igl = (nanoseconds >> 2); }

	inline unsigned int getLatency() { return (*igl) * 4; }

	inline void enable() {
		(*inte) = (*inte) | INTE_IG;
	}
	
	inline void pause() {
		latency = this->getLatency();
		this->setLatency(0);
	}

	inline void play() {
		this->setLatency(latency);
	}

	inline void disable() {
		(*inte) &= !(INTE_IG);
	}

	inline void ack() {
		(*igc) = CTRL_ACK;
	}
	
protected:
	const static unsigned int CTRL_RESET;	//** Reset Control word
	const static unsigned int CTRL_ACK;		//** Acknowledge Control word
	const static unsigned int INTE_IG;		//** Interrupt Enable bit

private:
	volatile puint igc;		//** Pointer to the Control register
	volatile puint igl;		//** Pointer to the Latency register
	volatile puint igan;	//** Pointer to the Assert Counter
	volatile puint igdn;	//** Pointer to the Deassert Counter
	volatile puint inte;	//** Pointer to the Interrupt Enable register
	volatile puint ints;	//** Pointer to the Interrupt Status register
	
	unsigned int latency;	//** Saved latency value. Used by the pause/continue commands.

};	// end class

} // end namespace

#endif /*INTERRUPTGENERATOR_H_*/
