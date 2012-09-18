/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * 
 * ---
 * From the Buffer Management Library:
 * 
 * Revision 1.4  2006/05/23 15:12:46  marcus
 * Added Windows Support
 *
 * Revision 1.3  2006/03/10 17:59:18  marcus
 * Added Pooled Buffer Manager support.
 *
 *******************************************************************/

#include "util/Timer.h"
#include <iostream>

using namespace mprace::util;

#ifdef _MSC_VER
 #include <windows.h>
#else
 #define POSIX_TIMER CLOCK_REALTIME
#endif

#define CALIBRATION_LOOPS 5

// Define Static vars
// CPUTicks per millisecond
clkticks_t Timer::ticks_per_ms;
bool Timer::_calibrated=false;

// Calibrate the Timer class for this system.
// Uses the POSIX functions to get a reliable measure.
void Timer::calibrate() {
#ifdef _MSC_VER
	LARGE_INTEGER ticks_per_sec;

	QueryPerformanceFrequency( &ticks_per_sec );
	Timer::ticks_per_ms = ticks_per_sec.QuadPart / 1000L;

	Timer::_calibrated = true;
	return;
#else
	clkticks_t acc = 0;
	int i;
	
	for(i=0; i < CALIBRATION_LOOPS; i++)
		acc += Timer::calibrate_loop();
	
	Timer::ticks_per_ms = acc / CALIBRATION_LOOPS;
	Timer::_calibrated = true;
	return;	
#endif
}

bool Timer::is_calibrated() { return Timer::_calibrated; }

clkticks_t Timer::calibrate_loop() {
#ifdef _MSC_VER
	/* This method does nothing.
	 * Windows get the calibration info from the frecuency 
	 */
	return 0;
#else
	/* use clock_nanosleep as a source for time calibration */
	/* measure ticks for 100 milliseconds*/
	struct timespec s,d,e,wt,rem;
	float diff;
	clkticks_t count;
	int i;
		
	diff = 0.0;
	
	wt.tv_sec = 0;
	wt.tv_nsec = 100000000L; /* 100ms */
	
	clock_gettime(POSIX_TIMER, &s);
	count = getCPUTicks();

	/* The loop is in case it is interrupted */		
	while (clock_nanosleep(POSIX_TIMER, 0, &wt, &rem) < 0)
		wt = rem;

	count = getCPUTicks() - count;
	clock_gettime(POSIX_TIMER, &e);

	/* we requested 100ms, but the result can be different */
	tsDiff( d, s, e );
	
	diff = tsAsSeconds(d)*1000;	/* as milliseconds */

	return static_cast<clkticks_t>(static_cast<float>(count)/diff);
#endif
}


void Timer::wait(float seconds) {
	Timer t;

	t.start();
	t.stop();
	while (t.asSeconds() < seconds )
		t.stop();
		
	return;	
}

void Timer::printCalInfo() {
#ifdef _MSC_VER
	LARGE_INTEGER res;
	QueryPerformanceFrequency( &res );

	std::cout << "QueryPerformanceFracuency resolution: " << Timer::ticks_per_ms << std::endl;
#else
	struct timespec tr;
	
	// get resolution of the timer used for calibration
	clock_getres(POSIX_TIMER, &tr);
	
	std::cout << "POSIX_TIMER resolution   : " << tsAsSeconds(tr) << " sec" << std::endl;
	std::cout << "CPUTicks per millisecond : " << Timer::ticks_per_ms << std::endl; 
	std::cout << "Calculated CPU Frecuency : " << Timer::ticks_per_ms / 1000 << " MHz" << std::endl;
#endif
}

#ifndef _MSC_VER
void Timer::tsDiff(struct timespec &diff, struct timespec& begin, struct timespec& end) {
	if (end.tv_nsec < begin.tv_nsec) {
		diff.tv_nsec = (end.tv_nsec + 1000000000L) - begin.tv_nsec;
		diff.tv_sec = end.tv_sec - begin.tv_sec - 1;
	}
	else {
		diff.tv_nsec = end.tv_nsec - begin.tv_nsec;
		diff.tv_sec = end.tv_sec - begin.tv_sec;
	}
	
	if (diff.tv_nsec > 1000000000L) {
		diff.tv_sec++;
		diff.tv_nsec -= 1000000000L;
	}
	
	return;
}

float Timer::tsAsSeconds(struct timespec &ts) {
	float f;
	f = static_cast<float>( ts.tv_sec );
	f += static_cast<float>(ts.tv_nsec) * 1E-9;
	return f;
}
#endif

float Timer::asSeconds() {
	return asMillis() / 1000;
}

float Timer::asMillis() {
	// TODO: Check for wrap around conditions. Check the RDTSC documentation for it.
	
	return static_cast<float>(stopTime-startTime) / ticks_per_ms;
}
