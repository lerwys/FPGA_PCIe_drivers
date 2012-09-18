/**
 * This is a test case to check the write and read
 * of the DMA-Buffer with variable offset and size
 *
 * @file testBuffersizes.cpp
 * @author Philipp Schaefer
 * @date 2010-07-13
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
#define FPGA_ADDR (0x8000 >> 2)

using namespace mprace;

static bool verbose = false;

int main(int argc, char *argv[]){

	srand(time(0));

	unsigned int offset = 0;
	unsigned int count = 1;
        unsigned int size = 8192;
	bool check_break = false;

        Board *board;
	board = new ABB(BOARD_NR);
	board->setReg(0x04, 0x0010);

	int c, option_index = 0;
	static struct option long_options[] = {
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};

	while ((c = getopt_long(argc, argv, "hvbs:o:c:", long_options, &option_index)) != -1) {
		switch (c) {
			case 'h':
				std::cout << "Syntax: " << argv[0] << " [-v] [-b] [-s] [-o] [-c]" << std::endl;
				std::cout << "-v\tPrint Buffers into file (error.txt, error-dma.txt" << std::endl;
				std::cout << "-b\tThe testcase breaks if it fails" << std::endl;
                                std::cout << "-s\tSet buffersize in dwords" << std::endl;
				std::cout << "-o\tSet offset in dwords" << std::endl;
				std::cout << "-c\tSet count in dwords" << std::endl;
				std::cout << "Default:\tTest with offset=0, buffersize=8192, count = 1" << std::endl;
				exit(EXIT_SUCCESS);
				break;
			case 'v':
				verbose = true;
				break;
			case 'b':
				check_break = true;
				break;
                        case 's':
                                size = atoi(optarg);
                                //std::cout << "Performing test with buffersize=" << size << std::endl;
                                break;
			case 'o':
				offset = atoi(optarg);
				//std::cout << "Performing test with offset=" << offset << std::endl;
				break;
			case 'c':
				count = atoi(optarg);
				//std::cout << "Performing test with count=" << count << std::endl;
				break;
			default:
				exit(EXIT_FAILURE);
				break;
		}
	}

	// we need a copy to check the buffer after DMA write/read
        int buf_copy[size];
	bool buf_equal = true;
	bool pio_dma = true;

        DMABuffer buf0(*board, (size * sizeof(int)), DMABuffer::USER);

	// we have to check that the right Exception is thrown, if we use
	// invalid values for count and offset
	for (int i = 0; i < size; i++)
		buf0[i] = 0x0C0000 + i + 1;  //rand();

	for (int i = 0; i < size; i++)
		buf_copy[i] = buf0[i];

	std::cout << "Try write-DMA with empty Transfer:" << std::endl;
	try{
		board->writeDMA(FPGA_ADDR, buf0, 0, offset, true, true);
	}
	catch(Exception EMPTY_TRANSFER)
	{
		std::cout << "Exception EMPTY_TRANSFER successfully catched" << std::endl;
	}

	std::cout << "Try write-DMA with Offset > Buffersize:" << std::endl;
	try{
		board->writeDMA(FPGA_ADDR, buf0, 0, size + 1, true, true);
	}
	catch(Exception ADDRESS_OUT_OF_RANGE)
	{
		std::cout << "Exception ADDRESS_OUT_OF_RANGE successfully catched" << std::endl;
	}

	std::cout << "Try write-DMA with Offset < 0:" << std::endl;
	try{
		board->writeDMA(FPGA_ADDR, buf0, -1, 0, true, true);
	}
	catch(Exception ADDRESS_OUT_OF_RANGE)
	{
		std::cout << "Exception ADDRESS_OUT_OF_RANGE successfully catched" << std::endl;
	}

	std::cout << "Try write-DMA with Count < 0:" << std::endl;
	try{
		board->writeDMA(FPGA_ADDR, buf0, 0, -1, true, true);
	}
	catch(Exception ADDRESS_OUT_OF_RANGE)
	{
		std::cout << "Exception ADDRESS_OUT_OF_RANGE successfully catched" << std::endl;
	}

	std::cout << "Try write-DMA with Offset + Count > Buffersize:" << std::endl;
	try{
		board->writeDMA(FPGA_ADDR, buf0, size/2, size/2 + 1, true, true);
	}
	catch(Exception ADDRESS_OUT_OF_RANGE)
	{
		std::cout << "Exception ADDRESS_OUT_OF_RANGE successfully catched" << std::endl;
	}

	std::cout << "Try write-DMA with Count > Buffersize:" << std::endl;
	try{
		board->writeDMA(FPGA_ADDR, buf0, size + 1, 0, true, true);
	}
	catch(Exception ADDRESS_OUT_OF_RANGE)
	{
		std::cout << "Exception ADDRESS_OUT_OF_RANGE successfully catched" << std::endl << std::endl;
	}

        std::cout << "Startvalues:" << std::endl;
        std::cout << "Offset: " << offset << std::endl;
        std::cout << "Count: " << count << std::endl;
        std::cout << "Buffersize: " << size << std::endl << std::endl;

	//test begins
	for (int j = count; j <= size; j++) {
		offset = 0;
		std::cout << "Testing with buffersize = " << size << ", transfersize = " << count << " and all valid offsets." << std::endl;
		for (int k = 0; k < size-j; k++) {

			//fill the buffer with random values and make a copy
			for (int i = 0; i < size; i++)
				buf0[i] = 0x0A0000 + i + 1; //rand();

			for (int i = 0; i < size; i++)
				buf_copy[i] = buf0[i];

			board->writeDMA(FPGA_ADDR, buf0, count, offset, true, true);

			//overwrite the buffer to ensure, that readDMA works
			for (int i = 0; i < size; i++)
				buf0[i] = 0xFF;

			//modify the copied buffer as we expect buf0 should be after readDMA
			for (int i = 0; i < offset; i++)
				buf_copy[i] = 0xFF;

			for (int i = offset + count; i < size; i++)
				buf_copy[i] = 0xFF;

			//writes the copied buffer in an errorfile
			if(verbose){
				std::ofstream output("error.txt", std::ios::out | std::ios::binary);

				output.write((char*)buf_copy,size);
			}

			board->readDMA(FPGA_ADDR, buf0, count, offset, true, true);

			//writes the read buffer in an errorfile
			if(verbose){
				int buf_write_copy[size];

				for (int i = 0; i < size; i++)
					buf_write_copy[i] = buf0[i];
				
				std::ofstream output1("error-dma.txt", std::ios::out | std::ios::binary);

				output1.write((char*)buf_write_copy,size);
			}

			//we ensure that the read buffer with dma equals the
			//read buffer with pio, so that we can be sure that the
			//buffer was read correctly
			for (int i = 0; i < count; i++){ 
				if (buf0[i+offset] != board->read(FPGA_ADDR + i))
				pio_dma = false;
			}
			
			//we check if the buffer was written and read correctly
			for (int i = 0; i < size; i++) {
				if (buf0[i] != buf_copy[i]) {
					printf("Offset: %d dec (0x%08x) \n", i,i);
					buf_equal = false;
					printf("read: 0x%08x expected: 0x%08x (%d dec) PIO: 0x%08x\n", buf0[i], buf_copy[i], buf_copy[i], board->read(FPGA_ADDR - offset + i));
					printf("register contents = %d %02x\n\n", board->getReg(0x26), board->getReg(0x26));
				}
			}

			if(!buf_equal) {
				if(!pio_dma){
					std::cout << "read-dma is not equal to read-pio" << std::endl;
					pio_dma = true;
				}
				buf_equal = true;
				std::cout << "Test failed with buffersize = " << size << " offset = " << offset << " transfersize = " << count << std::endl;
				if(check_break) {
					return 1;
				}
			}

			offset++;
		}

		if(buf_equal) {
			std::cout << "Test successed..." << std::endl;
			//return 0;
		}

		count++;
	}

	return 0;

}
