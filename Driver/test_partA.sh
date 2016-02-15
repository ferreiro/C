#!/bin/bash
# Create a kernel module and load it into the operating system      

MODULE_NAME="chardev_leds"
MODULE_PATH="/dev/"$MODULE_NAME

sudo rmmod $MODULE_NAME     # Unload the Kernel Module from the Operating System
sudo rm $MODULE_PATH        # Clean the driver folder recursively (if previous executions were run)
make clean                  # Clean previous compiled files
make                        # Recompile the source code of the program

sudo insmod $MODULE_NAME.ko # Load the module into the Operating system making it available for users to using and creating new driver's instance
lsmod | head                # Display device modules to see if our driver is already in the list.

sudo mknod -m 666 $MODULE_PATH c 250 0 # Create a character device driver (sudo mknod -m 666 <pathname_char_file> c <major> <minor>) 
                                       # “-m 666”: grant read/write permissions to everyone)

sudo echo 1 > $MODULE_PATH
dmesg | tail

sudo echo 21 > $MODULE_PATH
dmesg | tail

sudo echo 123 > $MODULE_PATH
dmesg | tail
