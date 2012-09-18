extern "C" {
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"
}

#include <mprace/Board.h>
#include <mprace/ABB.h>
#include <mprace/DMABuffer.h>

using namespace mprace;


MODULE = mprace		PACKAGE = Board

Board *
Board::new(char *type, int board_number)
CODE:
	if (strcmp(type, "ABB") == 0) {
		RETVAL = new ABB(board_number);
	} else {
		croak("Unknown board type");
	}
OUTPUT:
	RETVAL

void
Board::DESTROY()

int
Board::dmabuffer(int size)
CODE:
	printf("maw\n");
	printf("size is %d\n", size);
	RETVAL = 4;
OUTPUT:
	RETVAL

unsigned int
Board::getReg(int address)

void
Board::setReg(int address, int data)

void
Board::write(unsigned int address, unsigned int data)
