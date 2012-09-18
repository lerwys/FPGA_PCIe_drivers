#ifndef ICS8430I_H_
#define ICS8430I_H_

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2007/02/12 18:09:21  marcus
 * Initial commit.
 *
 *******************************************************************/

namespace mprace {

class Pin;

/**
 * Manages access to a ICS8430I-61 Frecuency Synthesizer.
 * 
 * 
 * @author  Guillermo Marcus
 * @version $Revision: 1.2 $
 * @date    $Date: 2007-05-29 07:50:42 $
 */

class ICS8430I {
public:
	/**
	 * Create a ICS8430I. This devices uses a custom 3-wire
	 * serial interface. See datasheet for a complete description.
	 * 
	 * @param S_CLOCK Pin associated with the clock pin.
	 * @param S_DATA Pin associated with the data pin.
	 * @param S_LOAD Pin associated with the load pin.
	 */
	ICS8430I( Pin& S_CLOCK, Pin& S_DATA, Pin& S_LOAD, const float fIn );

	/**
	 * Delete a ICS8430I.
	 */	
	~ICS8430I();

	/**
	 * Set the parameters for the device. See the datasheet
	 * for a complete description of the parameters ranges
	 * and meanings.
	 * 
	 * @param T Test output mode. Sets T1,T0.
	 * @param M VCO Frecuency. Sets M8-M0.
	 * @param N Output Divider. Sets N2-N0.
	 */
	void set(const unsigned int T, const unsigned int M, const unsigned int N);

	/**
	 * Get the actual Input frecuency /for computing frecuency only)
	 * @return Input frecuency, in Hz.
	 */
	float getInputFrecuency() { return fIn; }
	
	/**
	 * Set the Input Frecuency (for computing frecuency only)
	 * 
	 * @param value New input frecuency, in Hz.
	 */
	void setInputFrecuency(float value) {
		this->fIn = value;
		this->buildFrecTable();
	}
	
	/**
	 * Compute Output Frecuency based in the current settings.
	 * @return Computed frecuency in Hz, or -1.0 if does not lock.
	 */
	float getOutputFrecuency();
	
	/**
	 * Set the Output frecuency. This function scans the frecuency
	 * table to find the best match for M and N to use, for the current
	 * input frecuency.
	 * 
	 * @param f New output frecuency, in Hz
	 * @return True if set, false if not possible to find a valid combination.
	 */
	bool setOutputFrecuency(const float f);

private:
	const static float ts = 0.0;			/** Setup time */
	const static float th = 0.0;			/** Hold time */
	const static float fvcoMin = 250000000;	/** PLL fvco min. lock frecuency */
	const static float fvcoMax = 500000000; /** PLL fvco max. lock frecuency */
	const static float nDivider[];

	float frecTable[250][8];
	
	Pin *S_CLOCK;
	Pin *S_DATA;
	Pin *S_LOAD;
	
	float fIn;
	unsigned int m;
	unsigned int n;
		
	/**
	 * Send several bits. MSB bits are sent first.
	 * 
	 * @param width Number of bits to send
	 * @param value Integer containing the [ (width-1)..0 ] bits to send.
	 */
	void sendBits( const unsigned int width, const unsigned int value );

	/**
	 * Build the Frecuency table for allowed values of M and N.
	 * This table is used by the setFrecuency to compute the best matching
	 * values of M and N for the desired value.
	 */
	void buildFrecTable();

	
}; /* class ICS8430I */

} /* namespace mprace */

#endif /*ICS8430I_H_*/
