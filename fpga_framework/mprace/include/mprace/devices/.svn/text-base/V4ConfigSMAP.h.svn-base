#ifndef V4CONFIGSMAP_H_
#define V4CONFIGSMAP_H_

#include <string>
#include "../Board.h"

namespace mprace {

class Pin;
class Register;

class V4ConfigSMAP {
public:
	enum SMAPmode { SMAP8, SMAP32 };

	V4ConfigSMAP( Register *smap,
		SMAPmode mode,
		Pin *init_b,
		Pin *prog_b,
		Pin *cs_b,
		Pin *cclk,
		Pin *done,
		Pin *rdwr_b	 );

	~V4ConfigSMAP();
		
	Board::ConfigStatus config(const std::string& filename);
	
	Board::ConfigStatus config(const char* filename);

	Board::ConfigStatus config(unsigned int byteCount, const char* bitstream);

	inline Board::ConfigStatus getStatus() { return lastStatus; }

	inline void setMode( const SMAPmode mode ) { this->mode = mode; }
	
	inline SMAPmode getMode() { return this->mode; }

	inline void setVerbose(bool val) { verbose = val; }
	
	inline bool getVerbose() { return verbose; }

protected:
	Board::ConfigStatus lastStatus;
	SMAPmode mode;
	bool verbose;
	
	Register *smap;
	Pin *init_b;
	Pin *prog_b;
	Pin *cs_b;
	Pin *cclk;
	Pin *done;
	Pin *rdwr_b;

private:
	inline char byteHH( int val ) { return (((val) >> 24) & 0x000000FF); }	
	inline char byteLH( int val ) { return (((val) >> 16) & 0x000000FF); }	
	inline char byteHL( int val ) { return (((val) >> 8) & 0x000000FF); }	
	inline char byteLL( int val ) { return ((val) & 0x000000FF); }	

}; /* class V4ConfigSMAP */

} /* namespace mprace */

#endif /*V4CONFIGSMAP_H_*/
