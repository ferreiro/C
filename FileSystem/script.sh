#!/bin/bash

SRC="./src" # File with the source code from the project
TEMP="./temp" # temporary file
MOUNT="./mount-point" # Mount file

FILE1="myFS.h"
FILE2="fuseLib.c"
FILE3="MyFileSystem.c"

if [ $MOUNT ]; then
	rm -r $MOUNT/*
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

DIFF_FILE1=$(diff $MOUNT/$FILE1 $TEMP/$FILE1) # Make diff | Diffs: http://unix.stackexchange.com/questions/33638/diff-several-files-true-if-all-not-equal
DIFF_FILE2=$(diff $MOUNT/$FILE2 $TEMP/$FILE2)

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

# c: Check the integrity of the virtual disk again and perform a diff between the original file and the truncated one.

./my-fsck virtual-disk

DIFF_FILE1_TRUNCATED=$(diff $SRC/$FILE1 $MOUNT/$FILE1)

if [ "$DIFF_FILE1_TRUNCATED" == "" ]; 
then echo "[EQUALS] Original file and truncated are the same in MOUNT and TEMP"
else echo "[DIFFERENT] Original file and truncated are different in MOUNT and TEMP"
fi

# d: Copy a third text file into your FS.

cp $SRC/$FILE3 $MOUNT/$FILE3

# e: Check the integrity of the virtual disk and perform a diff between the original file and the one copied into the FS.

./my-fsck virtual-disk

DIFF_FILE3_COPIED=$(diff $SRC/$FILE3 $MOUNT/$FILE3)

if [ "$DIFF_FILE3_COPIED" == "" ]; 
then echo "[EQUALS] Third file copied on Mount is the same as in the Source."
else echo "[DIFFERENT] Third file copied on Mount is different as in the Source."
fi

# f: Truncate the second file in the temp folder and in your FS, so as to increase the file size in one block.

truncate --size=+4096 $TEMP/$FILE2
truncate --size=+4096 $MOUNT/$FILE2

# g: Check the integrity of the disk and perform a diff between the original file and the truncated one.

./my-fsck virtual-disk

DIFF_MOUNT_TRUNCATED_FILE2=$(diff $SRC/$FILE2 $MOUNT/$FILE2) # See Link1 at bottom | Make diff
DIFF_TEMP_TRUNCATED_FILE2=$(diff $SRC/$FILE2 $TEMP/$FILE2) # See Link1 at bottom | Make diff

if [ "$DIFF_MOUNT_TRUNCATED_FILE2" == "" ]; 
then echo "[EQUALS] Second file truncated in MOUNT is the same as the original source"
else echo "[DIFFERENT] Second file truncated in MOUNT is different from the original source"
fi

if [ "$DIFF_TEMP_TRUNCATED_FILE2" == "" ]; 
then echo "[EQUALS] Second file truncated in TEMP is the same as the original source"
else echo "[DIFFERENT] Second file truncated int TEMP is different from the original source"
fi
 
