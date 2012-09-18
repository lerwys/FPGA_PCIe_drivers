
#include "InterruptGenerator.h"

using namespace mprace;

const unsigned int InterruptGenerator::CTRL_RESET = 0x0A;	
const unsigned int InterruptGenerator::CTRL_ACK   = 0x00F0;	
const unsigned int InterruptGenerator::INTE_IG    = 0x00000004;	


InterruptGenerator::InterruptGenerator(unsigned int *base, unsigned int *INTE, unsigned int *INTS )
{
	inte = INTE;
	ints = INTS;
	igc = base;
	igl = base+1;
	igan = base+2;
	igdn = base+3;
	
	this->setLatency(0);
	this->disable();
}

InterruptGenerator::~InterruptGenerator()
{
	this->disable();
}
