/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2007/02/12 18:09:21  marcus
 * Initial commit.
 *
 *******************************************************************/

#include "devices/ICS8430I.h"
#include "Pin.h"
#include "util/Timer.h"
#include <cmath>

using namespace mprace;
using namespace mprace::util;

const float ICS8430I::nDivider[] = { 1.0, 1.5, 2.0, 3.0, 4.0, 6.0, 8.0, 12.0 };

ICS8430I::ICS8430I( Pin& S_CLOCK, Pin& S_DATA, Pin& S_LOAD, const float fIn ) {
	this->S_CLOCK = &S_CLOCK;
	this->S_DATA = &S_DATA;
	this->S_LOAD = &S_LOAD;
	this->setInputFrecuency(fIn);
}

ICS8430I::~ICS8430I() {
}

void ICS8430I::set(const unsigned int T, const unsigned int M, const unsigned int N) {
	
	// clear signals to start
	S_CLOCK->clear();
	S_LOAD->clear();

	// send configuration bits
	this->sendBits( 2, T );
	this->sendBits( 3, N );
	this->sendBits( 8, M );
	
	// Pulse load
	Timer::wait( ts );
	S_LOAD->set();
	Timer::wait( ts );
	S_LOAD->clear();

	// Save current values
	this->m = M;
	this->n = N;
	
	return;
}

void ICS8430I::sendBits( const unsigned int width, const unsigned int value ) {
	int i;
	unsigned int temp;

	// take out excess bits at the start
	temp = (value << (8*sizeof(unsigned int) - width));

	// send the bits
	for(i=0; i<width-1; i++) {
		if ( temp & 0x80000000 )
			S_DATA->set();
		else
			S_DATA->clear();
			
		Timer::wait( ts );
		
		S_CLOCK->set();
		Timer::wait( th );
		S_CLOCK->clear();
		
		temp = (temp << 1);
	}
	
	return;
}

float ICS8430I::getOutputFrecuency() {
	float fvco;
	float fout;
	
	fvco = fIn/16*m;
	
	// TODO: Shall I throw an exception here?
	if ((fvco < fvcoMin) || (fvco > fvcoMax))
		return -1.0;
	
	// Fetch divider from table and calculate frecuency
	fout = fvco / nDivider[n];
	
	return fout;
}

void ICS8430I::buildFrecTable() {
	int mi, ni;
	float fvco, fout;
	
	for(mi=0;mi<=250;mi++) {
		fvco = (fIn/16)*(mi+250);
		if ((fvco < fvcoMin) || (fvco > fvcoMax)) {
			for(ni=0;ni<8;ni++)
				frecTable[mi][ni] = -1.0;
			continue;
		}
		
		for(ni=0;ni<=7;ni++) {
			fout = fvco / nDivider[ni];
			frecTable[mi][ni] = fout;
		}
	}
	
	return;
}

bool ICS8430I::setOutputFrecuency(const float f) {
	unsigned int m_best=255, n_best=255;
	unsigned int mi, ni;
	float df_best = 2*fvcoMax;	// just something bigger than fvcoMax
	
	// Scan the table, and find the best M and N
	for(mi=0;mi<=250;mi++) {
		for(ni=0;ni<=8;ni++) {
			if ( fabs(f-frecTable[mi][ni]) < df_best) {
				m_best = mi;
				n_best = ni;
				df_best = fabs(f-frecTable[mi][ni]);
			}
		}
	}
	
	// No suitable value found
	if (df_best > fvcoMax)
		return false;
	
	// Set the appropiate values
	this->set(0, m_best+250, n_best);
	return true;
}
