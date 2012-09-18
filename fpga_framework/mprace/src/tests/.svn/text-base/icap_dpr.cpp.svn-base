#include <string>
#include <iostream>
#include <exception>
//#include <pthread.h>
#include <getopt.h>
#include <cstdlib>
#include <cstdio>
#include <stdint.h>

#include <fstream>

#include <mprace/Board.h>
#include <mprace/DMABuffer.h>
#include <mprace/ABB.h>
#include <mprace/util/Timer.h>

#include "mprace/util/Timer.h"



using namespace std;
using namespace mprace;
using namespace mprace::util;


#define BOARD_NR 	0

#ifdef OLD_REGISTERS
	#define FPGA_ADDR (0x0)
#else
	#define FPGA_ADDR (0x8000 >> 2)
#endif

#define NLOOPS		1
#define GCR_ADDR        (0xa)
#define ICAP_ADDR       (0x1f)

/** Global instance of the board */
Board *board;


/**
 *
 * Make the DPR boudary tri-state control by poking GCR register.
 *   bit[4] : upstream DMA module
 *   bit[5] : downstream DMA module
 *
 */
int main(int argc, char *argv[]) {
	ifstream Bit_File, count_file;
	long begin, end, bit_size;
	float  dpr_time = 3.14159;
	Timer t;

	int  toggle_value = 0;
	char w;
	int  v_fall, v_rise;
	int CSb, WRb;
	ifstream::pos_type l;


	/* calibrate Timers */
	mprace::util::Timer::calibrate();

	if (argc<=1) {
		cout << "Bit file name missing. Try again." << endl;
		return -1;
	}

	count_file.open (argv[1], ios::in|ios::binary);
	if (!count_file.is_open()){
		cout << "  Unable to open bit file." << endl;
		return -1;
	}
	begin = count_file.tellg();
	count_file.seekg (0, ios::end);
	end = count_file.tellg();
	count_file.close();

	Bit_File.open (argv[1], ios::in|ios::binary);
	if (!Bit_File.is_open()){
		cout << "  Unable to open bit file." << endl;
		return -1;
	}

//	cout << "Creating object for ABB board " << BOARD_NR << endl;
	board = new ABB(BOARD_NR);

//	toggle_value = board->getReg(GCR_ADDR);
//	cout << hex << "GCR status : 0x" << toggle_value << endl;

	cout << "DPR in process ..." << endl;
#if 1
	// close the boundary
	toggle_value = 0x0034;
	board->setReg(GCR_ADDR, toggle_value);
	cout << hex << "DPR boundary closed." << endl;
#endif
	t.start();

	// start loading bitfile via ICAP
	int i =0;
	CSb = 0;
	WRb = 0;
	l = 1;
	while(!Bit_File.eof()) { // && i<200){
		Bit_File.read (&w, l);
		i++;
		v_fall = (int)(w) & 0x00ff;
//		v_rise = (int)(w) & 0x00ff;
		v_fall += (1<<31);            // ICAP_CLK falls
//		v_rise += (1<<31) + (1<<19) + (CSb<<18) + (WRb<<17);  // ICAP_CLK rises
		board->setReg(ICAP_ADDR, v_fall);
//		board->setReg(ICAP_ADDR, v_rise);

//		if (i>96)
//		cout << hex << v_fall << " " << v_rise << " - " << hex << (unsigned int)(w) << endl;

	}
//	cout << endl;
//	board->setReg(ICAP_ADDR, 0x80060000);
//	board->setReg(ICAP_ADDR, 0x00060000);

	t.stop();

#if 1
	// open the boundary
	toggle_value = 0x0004;
	board->setReg(GCR_ADDR, toggle_value);
	cout << hex << "DPR boundary opened." << endl;
#endif

	Bit_File.close();
	bit_size = end - begin;
	dpr_time = t.asSeconds()*1000.0;

	cout << "DPR successfully finished." << endl;
	cout << "  Bit file name : " << argv[1] << endl;
	cout << "  Bit file size : " << dec << bit_size << " bytes" << endl;
	cout << "  Reconfiguration time = " << dpr_time << " ms"  << endl;
	cout << "  Reconfiguration speed = " << ((float)(bit_size)/dpr_time/1000.0) << " MB/s"  << endl;
	return 0;
}
