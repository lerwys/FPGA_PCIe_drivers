
#include "devices/V4ConfigSMAP.h"
#include "Pin.h"
#include "Register.h"
#include "Exception.h"
#include "Board.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>

using namespace mprace;
using namespace std;

V4ConfigSMAP::V4ConfigSMAP( Register *smap,
		SMAPmode mode,
		Pin *init_b,
		Pin *prog_b,
		Pin *cs_b,
		Pin *cclk,
		Pin *done,
		Pin *rdwr_b	 )
{
		this->mode = mode;
		this->smap = smap;
		this->init_b = init_b;
		this->prog_b = prog_b;
		this->cs_b = cs_b;
		this->cclk = cclk;
		this->done = done;
		this->rdwr_b = rdwr_b;
		this->lastStatus = Board::NOT_CONFIGURED;
		this->verbose = false;
		
		cclk->clear();
		cs_b->set();
		init_b->set();
		rdwr_b->set();
}

V4ConfigSMAP::~V4ConfigSMAP()
{
		delete smap;
		delete init_b;
		delete prog_b;
		delete cs_b;
		delete cclk;
		delete done;
		delete rdwr_b;
}


Board::ConfigStatus  V4ConfigSMAP::config(const std::string& filename)
{
	return this->config( filename.c_str() );
}
	
Board::ConfigStatus  V4ConfigSMAP::config(const char* filename)
{
	ifstream in;
	char *bitstream;
	unsigned int count;
	enum FileType { UNKNOWN, RBT, BIN, BIT };
	unsigned int type = UNKNOWN;	
	Board::ConfigStatus retval;
	
	in.open(filename);
	
	if (!in) {
		throw mprace::Exception( mprace::Exception::FILE_NOT_FOUND );
	}

	// determine the type of file
	// To do that, we check some of the characters of the stream.
	// BIN file: starts with 0x20 FF's, then has the signature AA995566
	// BIT file: starts with the sequence: 0x 00 09 0f f0 0f f0 0f f0 0f f0 00 01
	// RBT file: Text file, starts with the string "Xilinx ASCII Bitstream"

	// check for RBT type	
	char str[23];
	in.getline(str,23);
	string s(str);

	cout << str << endl;

	if (s.compare("Xilinx ASCII Bitstream") == 0) {
		type = RBT;
	}
	
	// check BIN type
	if (type == UNKNOWN) {
		unsigned char sig[4];
		// go to offset 0x20
		in.seekg( 0x20, ios_base::beg );
		in >> sig[0]
			>> sig[1]
			>> sig[2]
			>> sig[3];
		
		if ((sig[0] == 0xAA) && (sig[1] == 0x99) &&
			(sig[2] == 0x55) && (sig[3] == 0x66)) 
		{
			type = BIN;
		}
	}
	
	// check BIT type
	if (type == UNKNOWN) {
		// Bitfiles have a fixed start, then a section structure.
		// To validate, we read the fixed start only.
		
		unsigned char sig[12];
		unsigned char expected[12] = { 0x00, 0x09, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x00, 0x01 };
		int i,comp;
		
		// go to offset 0x6B
		in.seekg( 0x0, ios::beg );
		for(i=0;i<12;i++)
			in >> sig[i];
		
		comp=0;	
		for(i=0;i<12;i++)
			if (sig[i] != expected[i])
				comp=1;
		
		if (comp==0)
			type = BIT;
	}
	
	// if still unknown format, throw exception
	if (type == UNKNOWN) 
		throw mprace::Exception( mprace::Exception::UNKNOWN_FILE_FORMAT );

	// read the bitstream		
	switch (type) {
		case BIN:
			int size;
			in.seekg( 0, ios::end );
			count = in.tellg();
			in.seekg( 0, ios::beg );
			bitstream = new char[count];	
			in.get( bitstream, count );
			break;
		case BIT:
		{
			unsigned char temp;
			char *design_name;
			char *part_name;
			char *date_str;
			char *time_str;
			int   bitstream_size;
			unsigned short section_size;

			// We need to read all sections, to get the bitstream starting point			
			// Skip the first 13 bytes
			in.seekg( 13, ios::beg );
						
			// skip section header
			// get section size
			// allocate bytes for section
			// get section bytes

			// 'a': file name
			in >> temp;
			in >> section_size;
			design_name = new char[section_size];			
			in.get(design_name,section_size);

			// 'b': part name
			in >> temp;
			in >> section_size;
			part_name = new char[section_size];			
			in.get(part_name,section_size);

			// 'c': date string
			in >> temp;
			in >> section_size;
			date_str = new char[section_size];			
			in.get(date_str,section_size);

			// 'd': time string
			in >> temp;
			in >> section_size;
			time_str = new char[section_size];			
			in.get(time_str,section_size);

			// 'e': bitstream size
			in >> temp;
			in >> bitstream_size;
			
			// get bitstream
			bitstream = new char[bitstream_size];
			count = bitstream_size;
			in.get(bitstream,count);
		}
			break;
		case RBT:
		{
			// get ascii lines
			char header1[256];
			char bitstream_version[256];
			char design_name[256];
			char arch_name[256];
			char part_name[256];
			char date_str[256];
			char bit_size[256];
			char bitline[256];
			
			in.seekg( 0, ios::beg );
			
			in.getline( header1, 256 );
			in.getline( bitstream_version, 256 );
			in.getline( design_name, 256 );
			in.getline( arch_name, 256 );
			in.getline( part_name, 256 );
			in.getline( date_str, 256 );
			in.getline( bit_size, 256 );

			// calculate the number of bytes
			string tmp_str( bit_size );			
			int pos = tmp_str.find( 0x09, 0 );
			istringstream itmp_str( tmp_str );			

			itmp_str.seekg(pos);
			int bits, i;
			itmp_str >> bits;
			count = bits / 8;
			bitstream = new char[count];

			pos = 0;
			while ( in.getline(bitline, 256) ) {
				// process bit line
				int bitline_size = strlen(bitline);
				int val = 0;
				for(i=0;i<bitline_size;i++) {
					val = (val << 1);
					if (bitline[i] == '1')
						val += 1;
				}
				
				// set value in bitstream
				bitstream[pos++] = byteHH( val );
				bitstream[pos++] = byteLH( val );
				bitstream[pos++] = byteHL( val );
				bitstream[pos++] = byteLL( val );
			}
		}	
			break;
		default:
			throw mprace::Exception( mprace::Exception::UNKNOWN_FILE_FORMAT );
			break;
	}
	
	// get bitstream
	retval = config(count,bitstream);
	delete bitstream;
	return retval;
}

Board::ConfigStatus V4ConfigSMAP::config(unsigned int byteCount, const char* bitstream)
{
	int i,j;
	cout << hex;
	for(i=0;i<byteCount;i+=16) {
		cout.width(5);
		cout << i << ": ";
		cout.width(2);
		for(j=0;j<16;j++) {
			cout << bitstream[i];
			if (j != 15)
				cout << " ";
			else
				cout << endl;
		}
	}
	cout << dec;
	
	// Send bitstream
	if (mode == SMAP8) {
		// do bit reversal
	} else {
		// send bits as is (SMAP32)
	}
	
	
	// Do postcheck
	
	return Board::NOT_CONFIGURED;	
}

