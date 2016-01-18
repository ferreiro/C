#!/bin/bash       

MODULE_NAME="chardev_leds"
MODULE_PATH="/dev/"$MODULE_NAME

sudo rmmod $MODULE_NAME
sudo rm $MODULE_PATH # Clean generated file
make clean # clean the compilation files
make # recompile source code
sudo insmod $MODULE_NAME.ko # add module to the computer
lsmod | head # print the modules instantiate on the computer
sudo mknod -m 666 $MODULE_PATH c 250 0 # create a file module

echo "Tests!!!"

sudo echo 1 > $MODULE_PATH
dmesg | tail

sudo echo 21 > $MODULE_PATH
dmesg | tail

sudo echo 123 > $MODULE_PATH
dmesg | tail
