#ifndef TIMER_H_
#define TIMER_H_

/********************************************************************
 * A Timer class wraps and isolates the underlying method to
 * measure time accurately, namely, the Timestamp Counter Instruction 
 * in the x86 architecture. 
 * 
 * Please note that this class should NOT be used to measure 
 * long-running tasks. Use this class to measure milli-, micro- or 
 * nanoseconds accurately. For other needs, I reccomend to use 
 * 'clock_gettime' and 'clock_getres', with the POSIX timer of your
 * choice.
 * 
 * This class is based in the WinRealTimeClock class from the 
 * os/uelib. Due respects to Matthias Mueller and Christian Hinkelbein.
 * 
 * March 6th, 2006
 * Guillermo Marcus - Universitaet Mannheim
 * 
 * Originally developed for the Buffer Management library:
 *  Revision: 1.3
 *  Date: 2006/07/27 08:04:39
 * 
 * $Revision: 1.2 $
 * $Date: 2007-07-05 16:57:17 $
 * 
 *******************************************************************/

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2007/02/12 18:09:20  marcus
 * Initial commit.
 *
 * 
 * ---
 * From the Buffer Management Library:
 * 
 * Revision 1.3  2006/07/27 08:04:39  marcus
 * Corrected typo in the comments.
 *
 * Revision 1.2  2006/05/23 14:53:31  marcus
 * Added Windows Support
 *
 * Revision 1.1  2006/03/06 17:59:07  marcus
 * Initial BufferManager release.
 *
 *******************************************************************/


#include <ctime>
#ifdef _MSC_VER
 #include <windows.h>
#endif

// Namespace declarations
namespace mprace {
	namespace util {

typedef unsigned long long clkticks_t;

class Timer {
protected:
	static clkticks_t ticks_per_ms;
	static bool _calibrated;
	clkticks_t startTime;
	clkticks_t stopTime;

	// Get the CPU Ticks from the TimeStamp Counter 
	inline static clkticks_t getCPUTicks() {
#ifdef _MSC_VER
		LARGE_INTEGER val;
		QueryPerformanceCounter( &val );
		return val.QuadPart;
#else
		clkticks_t CPUTicks;
		unsigned int high,low;

		__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));

		CPUTicks=high;
		CPUTicks<<=32;
		CPUTicks+=low;

		return CPUTicks;
#endif
	}

	// internal
	static clkticks_t calibrate_loop();

#ifndef _MSC_VER
	// handy unix functions
	static void tsDiff(struct timespec &diff, struct timespec& start, struct timespec& end);
	static float tsAsSeconds(struct timespec& ts);
#endif

public:
	// calibrate the timer
	static void calibrate();
	static bool is_calibrated();
	static void printCalInfo();
	
	// A wait function
	static void wait(float seconds);
	
	// Set start time
	inline void start() { startTime = getCPUTicks(); }
	
	// Set stop time
	inline void stop() { stopTime = getCPUTicks(); }

	// Get elapsed time (stop-start) as seconds	
	float asSeconds();
	
	// Get elapsed time (stop-start) as milli seconds	
	float asMillis();
}; /* Timer class */

	} /* util namespace */
} /* mprace namespace */

#endif /*TIMER_H_*/
