#!/bin/bash
powerpc-linux-gcc client.c -o client2.out 
sudo cp client2.out /opt/nfsroot/root
echo DONE;
