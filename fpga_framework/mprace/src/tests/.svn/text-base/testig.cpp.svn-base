
#include <string>
#include <iostream>
#include <exception>
#include "mprace/Board.h"
#include "mprace/ABB.h"
#include "mprace/InterruptGenerator.h"
#include "mprace/Exception.h"

#ifdef FPGA
 #include "bufmgr/Timer.h"
#else
 #include "mprace/util/Timer.h"
#endif

using namespace mprace;
using namespace mprace::util;
using namespace std;

// Defines
#define BOARD_NR 0
#define NLOOPS 1
#define LATENCY_MIN 1000
#define LATENCY_MAX 1000000
const static bool verbose = false;

// Macros
#define asMB(x) (((double)(x))/(1024*1024))
#define asKB(x) ((double)(x)/1024)


bool testIG( ABB *board , unsigned int latency );
bool testIG2( ABB *board , unsigned int latency );

int main(int argc, char **argv) {

try {
	Timer::calibrate();

	ABB *board = new ABB(BOARD_NR);

	board->getDMAEngine().reset(0);
	board->getDMAEngine().reset(1);
	board->getInterruptGenerator().reset();
	board->setReg(ABB::Tx_CR, 0x0A);	// reset output queue
	
	cout << "Design ID: " << hex << board->getReg( ABB::DESIGN_ID ) << dec << endl;
	
	unsigned int latency = LATENCY_MIN;
	while (latency <= LATENCY_MAX) {		
		testIG2(board, latency );
		latency *= 2;
	}

	board->getInterruptGenerator().setLatency(0);
	board->getInterruptGenerator().reset();
	
} catch (exception& e) {
	cout << e.what() << endl;
}	
	return 0;
}

bool testIG(ABB *board, unsigned int latency)
{
	int i;
	InterruptGenerator& ig = board->getInterruptGenerator();
	Timer t;
	unsigned int dacnt;
	
	ig.reset();
	ig.enable();

	t.start();
	ig.setLatency(latency);	
	Timer::wait( 100*latency*1e-9 );
	ig.pause();
	t.stop();
	
	float wt= (latency < 10000) ? 1e-6 : (10*latency*1e-9);
	Timer::wait( wt );	
	dacnt = ig.getDeassertCount();
	
	char m;
	if (dacnt < ig.getAssertCount())
		m = '+';
	else
		m = ' ';
	
#if 0
	// complete ack
		while (ig.getDeassertCount() < ig.getAssertCount()) {
			Timer::wait( latency*1e-9 );
			ig.ack();
			
		}
#endif
	
	ig.disable();
		
	cout << latency << " ns\t" << ig.getAssertCount() << "\t" << dacnt << "\t" << m << "\t" << t.asMillis() << " ms" << endl;		
	
	return true;
}

bool testIG2(ABB *board, unsigned int latency)
{
	int i;
	InterruptGenerator& ig = board->getInterruptGenerator();
	Timer t;
	unsigned int dacnt;
	unsigned int count =0;
	
	ig.reset();
	ig.enable();
	ig.setLatency(latency);	

	t.start();
	while (count < 100) {
		board->waitForInterrupt(ABB::IRQ_IG);
		ig.enable();
		count++;
	}
	ig.pause();
	t.stop();
	
	dacnt = ig.getDeassertCount();
	
	char m;
	if (dacnt < ig.getAssertCount())
		m = '+';
	else
		m = ' ';
	
#if 0
	// complete ack
		while (ig.getDeassertCount() < ig.getAssertCount()) {
			Timer::wait( latency*1e-9 );
			ig.ack();
			
		}
#endif
	
	ig.disable();
		
	cout << latency << " ns\t" << ig.getAssertCount() << "\t" << dacnt << "\t" << m << "\t" << t.asMillis() << " ms" << endl;		
	
	return true;
}
