// testCppApi.cxx

// This is a copy from the old library CppApi interface test.
// If everything goes fine, this one must work transparently,
// except for the header to include.


//#include "cppApi/PciDevice.h"
//#include "cppApi/MemoryPageList.h"
#include "lib/pciDriver.h"

#include <unistd.h>
#include <iostream>
// NEVER MIX cout AND printf!!!!
#include <stdio.h> 

int pagesize = getpagesize();  

using namespace std;

int main () {

  unsigned int deviceNr = 0;
  unsigned int *buffer, *buffer2, *buffer3;
  unsigned int bufferSize = 5 * pagesize;
  unsigned int i;

  cout << "Allocating buffer" << endl;
  
  buffer = new unsigned int[ bufferSize];
  buffer2 = new unsigned int[ bufferSize];
  buffer3 = new unsigned int[ bufferSize];

  bufferSize = (4 * 5 * pagesize);
//  bufferSize = (4 * 5 * pagesize) - 1;

  if ((buffer == 0) || (buffer2 == 0) || (buffer3 == 0)) {
    cout << "Failed to alloc buffer" << endl;
    return -1;
  }

  cout << "Opening device: " << deviceNr << endl;

  PciDevice device;

  if (-1 == device.Open(deviceNr) ) {
    cout << "Failed opening device: " << deviceNr << endl;
  }
  cout << "OK" << endl;
  cout << "Dump PCI-config" << endl;

  for (i = 0; i < 0x10; i+=2) {
    cout << i 
	 << ": " 
	 << hex << (unsigned int) device.ReadConfigWord(i) << dec
	 << endl;
  }
  
  cout << endl;

  cout << "Bus: " << device.GetBus() << endl;
  cout << "Slot: " << device.GetSlot() << endl;

  cout << "Vendor ID: " << hex << device.GetVendorId() << endl;
  cout << "Device ID: " << hex << device.GetDeviceId() << endl;

  unsigned int * plx = (unsigned int *)  device.GetBarAccess(0);
  printf("query plx space\n");
  printf("plx:%p\n",plx);
  printf("DeviceID/VendorID: %#08x\n",*(plx+0x70/4));

  KMem *km;
  cout << "Testing Kernel Memory allocation" << endl;
  for(i=4;i<8;i++) {
  	cout << "allocating order " << i << endl;
  	km = new KMem( device, i );
  	cout << "Physical Address: " << km->GetPhysicalAddress() << endl;
  	cout << "Buffer Address: " << km->GetBuffer() << endl;
  	delete km;
  }
 

  MemoryPageList pageList;
  MemoryPageList pageList2;
  MemoryPageList pageList3;

  if (!pageList.LockBuffer(device, buffer, bufferSize)) {
    cout << "Failed to lock the buffer" << endl;
  }
  else {
    int index;

    cout << "Locking of buffer ok" << endl;

    for (index = 0; index < pageList.GetNumberOfPages(); index++) {
      cout << "Page: " 
	   << dec << index 
	   << "  Address: 0x" 
	   << hex << pageList[index] 
	   << endl;
    }
    
    cout << "Offset of the first page: 0x" 
	 << hex << pageList.GetFirstPageOffset() 
	 << dec  
	 << endl;

    cout << "Locking buffer2: " << endl;
    if (!pageList2.LockBuffer(device, buffer2, bufferSize)) {
      cout << "Failed to lock buffer 2" << endl;
    }
    else {

      cout << "Locked buffer 2" << endl;

      cout << "Locking buffer3: " << endl;
      if (!pageList3.LockBuffer(device, buffer3, bufferSize)) {
	cout << "Failed to lock buffer 3" << endl;
      }
      else {
	cout << "Locked buffer 3" << endl;
	if (!pageList3.UnlockBuffer()) {
	  cout << "Failed to unlock buffer 3" << endl;
	}
      }
      
      if (!pageList2.UnlockBuffer()) {
	cout << "Failed to unlock buffer 2" << endl;
      }
    }

    if (!pageList.UnlockBuffer()) {
      cout << "Failed to unlock the buffer" << endl;
    }
    else
      cout << "Buffer unlocked" << endl;
  }
/*  */
  device.Close();

  delete[] buffer;

  return 0;
}
