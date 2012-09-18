#include <iostream>
#include <mprace/Board.h>
#include <mprace/ABB.h>

using namespace std;
using namespace mprace;

int main() {
	Board *board = new ABB(0);
        cerr << "design: " << board->getReg(0x0) << endl;
}
