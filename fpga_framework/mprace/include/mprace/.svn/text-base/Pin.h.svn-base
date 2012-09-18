#ifndef PIN_H_
#define PIN_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2007/02/12 18:09:15  marcus
 * Initial commit.
 *
 *******************************************************************/

namespace mprace {

/**
 * Abstract interface to access a virtual pin.
 * 
 * A virtual pin is the basic building block for low-level device
 * interfaces. It provides a hardware abstraction layer for pin 
 * manipulation, separating the action (set/clear/get) from the platform
 * dependent access sequence (by example, accessing a register).
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.2 $
 * @date    $Date: 2007-05-29 07:50:44 $
 */

class Pin {
public:
	/**
	 * Delete a pin. Must be virtual to ensure the proper
	 * subclass destructor is called.
	 */
	virtual ~Pin() { };

	/**
	 * Set the pin to a certain value.
	 * @param value the new value.
	 * @exception mprace::Exception On error.
	 */
	virtual void set(bool value)=0;

	/**
	 * Get the pin current value.
	 * @return The current pin value.
	 * @exception mprace::Exception On error.
	 */
	virtual bool get()=0;
	
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
	/**
	 * Creates a Pin. Protected because only subclasses should 
	 * be instantiated.
	 */
	Pin() {};
	
	/* Avoid copy constructor, and copy assignment operator */

	/**
	 * Overrides default copy constructor, does nothing.
	 */
	Pin(const Pin&) {};

	/**
	 * Overrides default assignment operation, does nothing.
	 */
	Pin& operator=(const Pin&) { return *this; };

}; /* class Pin */

} /* namespace mprace */

#endif /*PIN_H_*/
