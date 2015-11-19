#!/bin/bash

SRC="./src" # File with the source code from the project
TEMP="./temp" # temporary file
MOUNT="./mount-point" # Mount file

FILE1="myFS.h"
FILE2="fuseLib.c"
FILE3="MyFileSystem.c"

if [ $MOUNT ]; then
	rm $MOUNT/*
	echo "[DELETED] Directory files from $MPOINT deleted... [OK]"
else
	mkdir $MOUNT # Create directory
fi

# a: copy 2 text files greater than 1 block (e.g., src/fuseLib.c and src/myFS.h) info MOUNT and TEMP.

cp $SRC/$FILE1 $TEMP/$FILE1	# copy source file to dest file
cp $SRC/$FILE1 $MOUNT/$FILE1

cp $SRC/$FILE2 $TEMP/$FILE2	
cp $SRC/$FILE2 $MOUNT/$FILE2

# b: Check integrity of the virtual disk with my-fsck and perform a diff between original files and those copied to the FS. 
#	 Truncate the first file (man truncate) in the MOUNT and TEMP folder so as to reduce the file size in one block.

./my-fsck virtual-disk

DIFF_FILE1=$(diff $MOUNT/$FILE1 $TEMP/$FILE1) # See Link1 at bottom | Make diff
DIFF_FILE2=$(diff $MOUNT/$FILE2 $TEMP/$FILE2) # See Link1 at bottom | Make diff

if [ "$DIFF_FILE1" == "" ]; 
then echo "[EQUALS] First file copied are same in MOUNT and TEMP"
else echo "[DIFFERENT] First file copied are different in MOUNT and TEMP"
fi

if [ "$DIFF_FILE2" == "" ]; 
then echo "[EQUALS] Second file copied are same in MOUNT and TEMP"
else echo "[DIFFERENT] Second file copied are different in MOUNT and TEMP"
fi

truncate --size=-4096 $TEMP/$FILE1
truncate --size=-4096 $MOUNT/$FILE1

echo "[TRUNCATED] File 1 has been truncated in TEMP and MOUNT"
./my-fsck virtual-disk

# Link1 - Diffs: http://unix.stackexchange.com/questions/33638/diff-several-files-true-if-all-not-equal
