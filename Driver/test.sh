#!/bin/bash       

MODULE_NAME="chardev_leds"
MODULE_PATH="/dev/"$MODULE_NAME

sudo rmmod $MODULE_NAME
sudo rm $MODULE_PATH # Clean generated file
make clean # clean the compilation files
make # recompile source code
sudo insmod $MODULE_NAME.ko # Load the module into the Operating system making available for users to use it and creating new drivers
lsmod | head # Display device modules that are in the operating System availaible currentyl.

# Create a character device driver (sudo mknod -m 666 <pathname_char_file> c <major> <minor>) 
sudo mknod -m 666 $MODULE_PATH c 250 0 # “-m 666”: grant read/write permissions to everyone)

echo "Tests!!!"

sudo echo 1 > $MODULE_PATH
dmesg | tail

sudo echo 21 > $MODULE_PATH
dmesg | tail

sudo echo 123 > $MODULE_PATH
dmesg | tail
