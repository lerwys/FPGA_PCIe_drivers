#ifndef REGISTER_H_
#define REGISTER_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 *******************************************************************/

namespace mprace {

/**
 * Abstract interface to access a virtual register.
 * 
 * A virtual register is the basic building block for low-level device
 * interfaces. It provides a hardware abstraction layer for register 
 * manipulation, separating the action (set/clear/get) from the platform
 * dependent access sequence (by example, accessing a memory address, or querying a device).
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.1 $
 * @date    $Date: 2007-05-29 07:50:49 $
 */

class Register {
public:
	/**
	 * Delete a Register. Must be virtual to ensure the proper
	 * subclass destructor is called.
	 */
	virtual ~Register() {};

	/**
	 * Set the register to a certain value.
	 * @param value the new value.
	 * @exception mprace::Exception On error.
	 */
	virtual void set(const unsigned int value)=0;

	/**
	 * Get the register current value.
	 * @return The current register value.
	 * @exception mprace::Exception On error.
	 */
	virtual unsigned int get()=0;
	
	/**
	 * Set the register to all ones.
	 * @exception mprace::Exception On error.
	 */
	inline void set() { this->set( 0xFFFFFFFF ); }

	/**
	 * Set the register to all zeros.
	 * @exception mprace::Exception On error.
	 */
	inline void clear() { this->set( 0x00000000 ); }

protected:
	/**
	 * Creates a Register. Protected because only subclasses should 
	 * be instantiated.
	 */
	Register() {};
	
	/* Avoid copy constructor, and copy assignment operator */

	/**
	 * Overrides default copy constructor, does nothing.
	 */
	Register(const Register&) {};

	/**
	 * Overrides default assignment operation, does nothing.
	 */
	Register& operator=(const Register&) { return *this; };

}; /* class Register */

} /* namespace mprace */

#endif /*REGISTER_H_*/
