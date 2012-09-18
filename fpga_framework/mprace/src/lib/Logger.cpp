/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 *******************************************************************/

#include "Logger.h"
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace mprace;
using namespace std;

Logger::Logger() {
	// TODO: Implement me!
	this->openLog();
	this->openDebugLog();
}

Logger::~Logger() {
	// TODO: Implement me!
	delete log;
	delete debugLog;
}

void Logger::openLog() {
	// TODO: Implement me!
	log = new ofstream("mprace.log");
}

void Logger::openDebugLog() {
	// TODO: Implement me!
	debugLog = new ofstream("mprace_debug.log");
}

void Logger::logEntry(const std::string& s) {
	if (verbose > 0)
		(*log) << s << endl;
}

void Logger::logDebugEntry(const std::string& s) {
	if (verbose > 1)
		(*debugLog) << s << endl;
}

void Logger::write(const unsigned int address, const unsigned int value) {
	if (verbose > 0)
		(*log) << setw(8) << setfill('0') << "Single Write "  << hex << address << " - " << value << endl;
	
	if (verbose > 1)
		(*debugLog) << setw(8) << setfill('0') << "Single Write " << hex << address << " - " << value << endl;
}

void Logger::read(const unsigned int address, const unsigned int value) {
	if (verbose > 0)
		(*log) << setw(8) << setfill('0') << "Single Read " << hex << address << " - " << value << endl;

	if (verbose > 1)
		(*debugLog) << setw(8) << setfill('0') << "Single Read " << hex << address << " - " << value << endl;
}

void Logger::writeBlock(const unsigned int address, const unsigned int *startPtr, const unsigned int count) {
	if (verbose > 0)
		(*log) << setw(8) << setfill('0') << "Block Write "  << hex << address << " - " << dec << count << " words - " << "block[] " << hex << startPtr << endl;
	
	if (verbose > 1) {
		int i;
		
		(*debugLog) << setw(8) << setfill('0') << "Block Write " << hex << address << " - " << dec << count << endl;
		for(i=0;i<count;i++) {
			(*debugLog) << setw(8) << setfill('0') << hex << (startPtr+i) << "  " << *(startPtr+i) << endl;
		}
	}
}

void Logger::readBlock(const unsigned int address, const unsigned int *startPtr, const unsigned int count) {
	if (verbose > 0)
		(*log) << setw(8) << setfill('0') << "Block Read "  << hex << address << " - " << dec << count << " words - " << "block[] " << hex << startPtr << endl;

	if (verbose > 1) {
		int i;
		
		(*debugLog) << setw(8) << setfill('0') << "Block Read " << hex << address << " - " << dec << count << endl;
		for(i=0;i<count;i++) {
			(*debugLog) << setw(8) << setfill('0') << hex << (startPtr+i) << "  " << *(startPtr+i) << endl;
		}
	}
}
