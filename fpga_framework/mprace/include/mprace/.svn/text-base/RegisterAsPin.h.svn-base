#ifndef REGISTERASPIN_H_
#define REGISTERASPIN_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2007/03/13 16:17:29  marcus
 * Initial Release.
 *
 * Revision 1.1  2007/02/12 18:09:17  marcus
 * Initial commit.
 *
 *******************************************************************/

namespace mprace {

class Pin;

/**
 * In this implementation, a single address (a register) acts as a
 * single pin. Writing sets all the bits to the requested value,
 * reading returns the value of the bit 0.
 * 
 * This allows for a very easy pin mapping, where each pin has a
 * separate register associated with it.
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.1 $
 * @date    $Date: 2007-05-29 07:50:44 $
 */

class RegisterAsPin : public Pin {
public:
	/**
	 * Creates a Pin associated with the specified address.
	 * @param reg The memory address where the associated register is mapped.
	 */
	RegisterAsPin(unsigned int *reg) {
		this->reg = reg;
	}

	/**
	 * Delete a pin. Must be virtual to ensure the proper
	 * subclass destructor is called.
	 */
	virtual ~RegisterPin() { }

	/**
	 * Set the pin to a certain value.
	 * @param value the new value.
	 * @exception mprace::Exception On error.
	 */
	virtual void set(bool value) { *(this->reg) = (value) ? 0xFFFFFFFF : 0x0; }

	/**
	 * Get the pin current value.
	 * @return The current pin value.
	 * @exception mprace::Exception On error.
	 */
	virtual bool get() { return (*(this->reg) != 0x0); }
	
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
	RegisterAsPin(const RegisterAsPin&) {};

	/**
	 * Overrides default assignment operation, does nothing.
	 */
	RegisterAsPin& operator=(const RegisterAsPin&) { return *this; };

private:
	unsigned int *reg;
	
}; /* class RegisterAsPin */

} /* namespace mprace */

#endif /*REGISTERASPIN_H_*/
