#ifndef REGISTERTRISTATE_H_
#define REGISTERTRISTATE_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 *******************************************************************/

namespace mprace {

class Register;

/**
 * In this implementation, 3 addresses are used for a single
 * register. One is used for reading, another for writing,
 * and a third one as output enable.
 * Writing sets the bits to the requested value, reading returns 
 * the value.
 * 
 * This allows for an easy register mapping which is hardware friendly, 
 * allowing easy control of bit direction in a register.
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.1 $
 * @date    $Date: 2007-05-29 07:50:45 $
 */

class RegisterTristate : public Register {
public:
	/**
	 * Creates a Register associated with the specified addresses.
	 * @param in_reg The memory address associated with the input register.
	 * @param out_reg The memory address associated with the output register.
	 * @param oe_reg The memory address associated with the OE register.
	 */
	RegisterTristate(unsigned int *in_reg, unsigned int *out_reg, unsigned int *oe_reg) { 
		in = in_reg;
		out = out_reg;
		oe = oe_reg;		
	}

	/**
	 * Delete a register. Must be virtual to ensure the proper
	 * subclass destructor is called.
	 */
	virtual ~RegisterTristate() { }

	/**
	 * Set the pin to a certain value.
	 * @param value the new value.
	 * @exception mprace::Exception On error.
	 */
	inline virtual void set(const unsigned int value) 
	{
		*(this->out) = (value); 
	}

	/**
	 * Enable the output for all bits.
	 * @exception mprace::Exception On error.
	 */
	inline virtual void setOE() 
	{ 
		*(this->oe) = 0xFFFFFFFF; 
	}

	/**
	 * Enable the output for certain bits.
	 * @exception mprace::Exception On error.
	 */
	inline virtual void setOE(const unsigned int value) 
	{ 
		*(this->oe) = value; 
	}

	/**
	 * Get the register current value.
	 * @return The current pin value.
	 * @exception mprace::Exception On error.
	 */
	inline virtual unsigned int get() { return *(this->in); }
	
	/**
	 * Set all bits to true.
	 * @exception mprace::Exception On error.
	 */
	inline void set() { this->set( 0xFFFFFFFF ); }

	/**
	 * Set all bits to false.
	 * @exception mprace::Exception On error.
	 */
	inline void clear() { this->set( 0x00000000 ); }

protected:
	/* Avoid copy constructor, and copy assignment operator */

	/**
	 * Overrides default copy constructor, does nothing.
	 */
	RegisterTristate(const Register&) {};

	/**
	 * Overrides default assignment operation, does nothing.
	 */
	RegisterTristate& operator=(const RegisterTristate&) { return *this; };

private:
	unsigned int *in;
	unsigned int *out;
	unsigned int *oe;
	bool oe_set;
	
}; /* class RegisterTristate */

} /* namespace mprace */

#endif /*REGISTERTRISTATE_H_*/
