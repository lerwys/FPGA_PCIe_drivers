#ifndef PININREGISTER_H_
#define PININREGISTER_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 *******************************************************************/

#include "Register.h"

namespace mprace {

class Pin;

/**
 * In this implementation, a pin is a bit in an existing register. 
 * Writing sets the associated bit in the register, reading returns its value.
 * 
 * This allows for a very easy pin mapping, with multiple bits mapped
 * into a single register.
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.1 $
 * @date    $Date: 2007-05-29 07:50:47 $
 */

class PinInRegister : public Pin {
public:
	/**
	 * Creates a Pin associated with the specified addresses.
	 * @param reg The memory address where the associated register is mapped.
	 */
	PinInRegister(Register& reg, unsigned int bit) {
		this->reg = &reg;
		this->bit = bit;
		this->bitMask = mask[bit];
	}

	/**
	 * Delete a pin. Must be virtual to ensure the proper
	 * subclass destructor is called.
	 */
	virtual ~PinInRegister() { }

	/**
	 * Set the pin to a certain value.
	 * @param value the new value.
	 * @exception mprace::Exception On error.
	 */
	virtual void set(bool value) 
	{
		if (value)
			reg->set( reg->get() | bitMask );
		else
			reg->set( reg->get() & !bitMask );
	}

	/**
	 * Get the pin current value.
	 * @return The current pin value.
	 * @exception mprace::Exception On error.
	 */
	virtual bool get() { return (reg->get() & bitMask) != 0x0; }
	
	/**
	 * Set the pin to true.
	 * @exception mprace::Exception On error.
	 */
	inline void set() { this->set( true ); }

	/**
	 * Set the pin to false.
	 * @exception mprace::Exception On error.
	 */
	inline void clear() { this->set( false ); }

protected:
	/* Avoid copy constructor, and copy assignment operator */

	/**
	 * Overrides default copy constructor, does nothing.
	 */
	PinInRegister(const PinInRegister&) {};

	/**
	 * Overrides default assignment operation, does nothing.
	 */
	PinInRegister& operator=(const PinInRegister&) { return *this; };

private:
	Register *reg;
	unsigned int bit;
	unsigned int bitMask;

	static const unsigned int mask[]; 
	
}; /* class PinInRegister */

const unsigned int PinInRegister::mask[] = {
			0x00000001,		//>** bit 0
			0x00000002,		//>** bit 1
			0x00000004,		//>** bit 2
			0x00000008,		//>** bit 3
			0x00000010,		//>** bit 4
			0x00000020,		//>** bit 5
			0x00000040,		//>** bit 6
			0x00000080,		//>** bit 7
			0x00000100,		//>** bit 8
			0x00000200,		//>** bit 9
			0x00000400,		//>** bit 10
			0x00000800,		//>** bit 11
			0x00001000,		//>** bit 12
			0x00002000,		//>** bit 13
			0x00004000,		//>** bit 14
			0x00008000,		//>** bit 15
			0x00010000,		//>** bit 16
			0x00020000,		//>** bit 17
			0x00040000,		//>** bit 18
			0x00080000,		//>** bit 19
			0x00100000,		//>** bit 20
			0x00200000,		//>** bit 21
			0x00400000,		//>** bit 22
			0x00800000,		//>** bit 23
			0x01000000,		//>** bit 24
			0x02000000,		//>** bit 25
			0x04000000,		//>** bit 26
			0x08000000,		//>** bit 27
			0x10000000,		//>** bit 28
			0x20000000,		//>** bit 29
			0x40000000,		//>** bit 30
			0x80000000 		//>** bit 31
		};


} /* namespace mprace */

#endif /*PININREGISTER_H_*/
