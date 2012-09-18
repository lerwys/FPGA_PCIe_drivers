#ifndef REGISTERASPINTRISTATE_H_
#define REGISTERASPINTRISTATE_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2007/03/13 16:17:29  marcus
 * Initial Release.
 *
 *******************************************************************/

namespace mprace {

class Pin;

/**
 * In this implementation, 3 addresses (registers) are used for a
 * single pin. One register is used for reading, another for writing,
 * and a third one as output enable.
 * Writing sets all the bits to the requested value, reading returns 
 * the value of the bit 0.
 * 
 * This allows for an easy pin mapping which is hardware friendly, 
 * where each pin has a separate register associated with it.
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.1 $
 * @date    $Date: 2007-05-29 07:50:49 $
 */

class RegisterAsPinTristate : public Pin {
public:
	/**
	 * Creates a Pin associated with the specified address.
	 * @param in_reg The memory address associated with the input register.
	 * @param out_reg The memory address associated with the output register.
	 * @param oe_reg The memory address associated with the OE register.
	 */
	RegisterAsPinTristate(unsigned int *in_reg, unsigned int *out_reg, unsigned int *oe_reg) { 
		in = in_reg;
		out = out_reg;
		oe = oe_reg;
		
		// In this implementation, the output is activated on the first set.
		*(this->oe_reg) = 0x0;
	}

	/**
	 * Delete a pin. Must be virtual to ensure the proper
	 * subclass destructor is called.
	 */
	virtual ~RegisterAsPinTristate() { }

	/**
	 * Set the pin to a certain value.
	 * @param value the new value.
	 * @exception mprace::Exception On error.
	 */
	inline virtual void set(bool value) 
	{
		if (!oe_set)
			this->setOE();
		
		*(this->out_reg) = (value) ? 0xFFFFFFFF : 0x0; 
	}

	/**
	 * Enable the output for the pin.
	 * @exception mprace::Exception On error.
	 */
	inline virtual void setOE() 
	{ 
		*(this->oe_reg) = 0xFFFFFFFF; 
		oe_set = true;
	}

	/**
	 * Get the pin current value.
	 * @return The current pin value.
	 * @exception mprace::Exception On error.
	 */
	inline virtual bool get() { return (*(this->in_reg) != 0x0); }
	
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
	RegisterAsPinTristate(const RegisterPin&) {};

	/**
	 * Overrides default assignment operation, does nothing.
	 */
	RegisterAsPinTristate& operator=(const RegisterAsPinTristate&) { return *this; };

private:
	unsigned int *in;
	unsigned int *out;
	unsigned int *oe;
	bool oe_set;
	
}; /* class RegisterAsPinTristate */

} /* namespace mprace */

#endif /*REGISTERASPINTRISTATE_H_*/
