/**
 * This is a test case to ensure that DMAEngineWG::sendDescriptorList works
 * correctly. The idea is to fragment the memory as much as possible, so that
 * even a 8KB buffer (the maximum buffer of our card) is allocated on different
 * physical addresses.
 *
 * @file testSendDescriptor.cpp
 * @author Philipp Schaefer
 * @date 2010-08-10
 *
 */


#include <iostream>
#include <string>
#include <fstream>
#include <exception>
#include <getopt.h>
#include <cstdlib>
#include <cstdio>
#include <stdint.h>
#include <time.h>

#include "mprace/DMABuffer.h"
#include "mprace/DMAEngine.h"
#include "mprace/DMAEngineWG.h"
#include "mprace/DMADescriptor.h"
#include "mprace/DMADescriptorWG.h"
#include "mprace/DMADescriptorListWG.h"
#include "mprace/Board.h"
#include "mprace/ABB.h"
#include "mprace/Driver.h"
#include "mprace/Exception.h"
#include "mprace/PCIDriver.h"
#include "mprace/util/Timer.h"
#include "pciDriver/lib/pciDriver.h"

#define BOARD_NR 0
#ifdef OLD_REGISTERS
	#define FPGA_ADDR (0x0)
#else
	#define FPGA_ADDR (0x8000 >> 2)
#endif

//#define USERTEST
#define KERNELPIECESTEST

using namespace mprace;

static bool verbose = false;
static bool loop = true;
bool buf_equal = true;
bool break_on_error = false;

unsigned int offset = 0;
unsigned int count = 2;
unsigned int max_desc = 0;

#ifdef USERTEST
unsigned int spam_size = 1024*1024;
unsigned int size = 2048*1024;
#endif

#ifdef KERNELPIECESTEST
unsigned int size = 16*1024;
unsigned int kernelpieces = 2;
#endif

void print_desc(DMABuffer& buf0) {

	DMADescriptorList *dliste = &buf0.getDescriptors();
	DMADescriptorListWG& liste = *(static_cast<DMADescriptorListWG*>(dliste));

	//we try to find out how many descriptors we really need for the transfer
	unsigned int used_desc_count = 0;
	unsigned int init_desc = 0;
	unsigned int cur_desc = 0;
	unsigned int last_desc = 0;
	unsigned int byte_count = 0;
	unsigned int byte_offset = offset*4;
	unsigned int block_offset = 0;

	while (init_desc < liste.getSize()) {
		if (byte_offset > byte_count + liste[init_desc].getLength()) {
			byte_count += liste[init_desc].getLength();
			init_desc++;
		}
		else {
			block_offset = byte_offset - byte_count;
			break;
		}
	}

	byte_count = count*4;
	cur_desc = init_desc;
	bool fits_in_desc;

	while (1) {
		for (int i = 0; i < liste.getSize() - init_desc; i++) {
			if (cur_desc == init_desc)
				fits_in_desc = (byte_count <= liste[init_desc].getLength() - block_offset);
			else fits_in_desc = (byte_count <= liste[cur_desc].getLength());

			if (fits_in_desc) {
				break;
			}
			else {	
				if (cur_desc == init_desc){
					byte_count -= (liste[init_desc].getLength() - block_offset);
				}
				else byte_count -= liste[cur_desc].getLength();
				cur_desc++;
			}
		}
		break;
	}

	last_desc = cur_desc;
	used_desc_count = last_desc - init_desc + 1;


	if (verbose) {
		std::cout << "Number of descriptors: " << liste.getSize() << std::endl;
		std::cout << "Number of used descriptors: " << used_desc_count << std::endl;
		std::cout << "INIT DESC " << init_desc << std::endl;
		std::cout << "LAST DESC " << last_desc << std::endl;
		/*for (int i = 0; i < liste.getSize(); i++){
			printf("Length of Descriptor %i: 0x%x \n", i, liste[i].getLength());
		}*/
		std::cout << "------------------------------ DescriptorList --------------------------" << std::endl;
		liste.print();
	}
	
	if(max_desc < used_desc_count)
		max_desc = used_desc_count;

}


int main(int argc, char *argv[]){

	srand(time(0));

	Board *board;
	board = new ABB(BOARD_NR);
	board->setReg(0x04, 0x0010);

	int c, option_index = 0;
	static struct option long_options[] = {
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};

	while ((c = getopt_long(argc, argv, "hvlbs:o:c:k:", long_options, &option_index)) != -1) {
		switch (c) {
			case 'h':
				std::cout << "Syntax: " << argv[0] << " [-h] [-v] [-o] [-c] [-s] [-b] [-k]" << std::endl;
				std::cout << "\n-o\t set offset" << std::endl;
				std::cout << "-c\t set transfer size" << std::endl;
				std::cout << "-s\t set size of the buffer" << std::endl;
				std::cout << "-b\t break on error" << std::endl;
				std::cout << "-k\t set number of kernelpieces" << std::endl;
				std::cout << "\ndefault values" << std::endl;
				std::cout << "offset:\t" << offset << std::endl;
				std::cout << "count:\t" << count << std::endl;
				std::cout << "size:\t" << size << " byte" <<  std::endl;
#ifdef KERNELPIECESTEST
				std::cout << "kernel_pieces: \t" << kernelpieces << std::endl;
#endif
				exit(EXIT_SUCCESS);
				break;
			case 'v':
				verbose = true;
				break;
			case 'l':
				loop = false;
				break;
			case 'b':
				break_on_error = true;
				break;
			case 's':
				size = atoi(optarg);
				break;
			case 'o':
				offset = atoi(optarg);
				break;
			case 'c':
				count = atoi(optarg);
				break;
			case 'k':
#ifdef KERNELPIECESTEST
				kernelpieces = atoi(optarg);
#endif
				break;
			default:
				exit(EXIT_FAILURE);
				break;
		}
	}

	int * buf_copy;
	buf_copy = (int *) malloc(sizeof(int) * size);

#ifdef USERTEST

	DMABuffer buf_spam1(*board, (spam_size * sizeof(int)), DMABuffer::USER);
//        DMABuffer buf_spam2(*board, (spam_size * sizeof(int)), DMABuffer::USER);
//        DMABuffer buf_spam3(*board, (spam_size * sizeof(int)), DMABuffer::USER);
	DMABuffer buf_spam4(*board, (spam_size * sizeof(int)), DMABuffer::USER);
	DMABuffer buf_spam5(*board, (spam_size * sizeof(int)), DMABuffer::USER);
//        DMABuffer buf_spam6(*board, (spam_size * sizeof(int)), DMABuffer::USER);
	DMABuffer buf_spam7(*board, (spam_size * sizeof(int)), DMABuffer::USER);
	DMABuffer buf_spam8(*board, (spam_size * sizeof(int)), DMABuffer::USER);
	DMABuffer buf_spam9(*board, (spam_size * sizeof(int)), DMABuffer::USER);
	DMABuffer buf_spam10(*board, (spam_size * sizeof(int)), DMABuffer::USER);
//        DMABuffer buf_spam11(*board, (spam_size * sizeof(int)), DMABuffer::USER);
	DMABuffer buf_spam12(*board, (spam_size * sizeof(int)), DMABuffer::USER);
	DMABuffer buf_spam13(*board, (spam_size * sizeof(int)), DMABuffer::USER);
	DMABuffer buf_spam14(*board, (spam_size * sizeof(int)), DMABuffer::USER);
//        DMABuffer buf_spam15(*board, (spam_size * sizeof(int)), DMABuffer::USER);
	DMABuffer buf_spam16(*board, (spam_size * sizeof(int)), DMABuffer::USER);
	DMABuffer buf_spam17(*board, (spam_size * sizeof(int)), DMABuffer::USER);
//        DMABuffer buf_spam18(*board, (spam_size * sizeof(int)), DMABuffer::USER);
//        DMABuffer buf_spam19(*board, (spam_size * sizeof(int)), DMABuffer::USER);
	DMABuffer buf_spam20(*board, (spam_size * sizeof(int)), DMABuffer::USER);
	
	buf_spam20.DMABuffer::~DMABuffer();
	buf_spam10.DMABuffer::~DMABuffer();
	buf_spam4.DMABuffer::~DMABuffer();
	buf_spam7.DMABuffer::~DMABuffer();
	buf_spam1.DMABuffer::~DMABuffer();
	buf_spam12.DMABuffer::~DMABuffer();
#endif

#ifdef USERTEST
	DMABuffer buf0(*board, (size * sizeof(int)), DMABuffer::USER);
#endif

#ifdef KERNELPIECESTEST
	DMABuffer buf0(*board, (size * sizeof(int)), DMABuffer::KERNEL_PIECES, kernelpieces);
#endif

	for (int i = 0; i < size; i++)
		buf0[i] = i; //0x0C000 + i + 1;

	for (int i = 0; i < size; i++)
		buf_copy[i] = buf0[i];

	std::cout << "Testing with count = " << count << " size = " << size << " offsets = " << offset << " and number of Kernelpieces " << kernelpieces << std::endl;
	board->writeDMA(FPGA_ADDR, buf0, count, offset, true, true);
	
	for (int i = 0; i < size; i++)
		buf0[i] = 0xFF;
	for (int i = 0; i < offset; i++)
		buf_copy[i] = 0xFF;
	for (int i = offset + count; i < size; i++)
		buf_copy[i] = 0xFF;

//				std::cout << "offset " << offset << " count " << count << " size " << size << std::endl;
	board->readDMA(FPGA_ADDR, buf0, count, offset, true, true);
//				std::cout << "-----------------------------------------------" << std::endl;
//				std::cout << "read successfull" << std::endl;

	buf_equal = true;

	bool pio_dma = true;

	for (int i = 0; i < count; i++) {
		if (buf0[i] != buf_copy[i]) {
			buf_equal = false;
			printf("\n\nWrong value on position: %d dec (0x%08x) \n", i,i);
			printf("read: 0x%08x expected: 0x%08x (%d dec) pio-read: 0x%08x \n", buf0[i], buf_copy[i], buf_copy[i], board->read(FPGA_ADDR - offset +i));
			printf("register contents = %d %02x\n\n", board->getReg(0x26), board->getReg(0x26));
		}
	}

	if (!buf_equal) {
		std::cout << "Test failed with offset = " << offset << " and count = " << count << std::endl;
		verbose = true;
		print_desc(buf0);
		return 1;
	}

	if (buf_equal) {
		std::cout << "Test successed..." << std::endl;
	}
	
	else {
		std::cout << "Test failed..." << std::endl;
	}
	return 0;
}
