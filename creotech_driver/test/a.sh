#!/bin/bash
powerpc-linux-gcc ioctl.c -o ioctl.out 
sudo cp ioctl.out /opt/nfsroot/root
echo DONE ioctl;
