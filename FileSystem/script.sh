#!/bin/bash

TMP="./temp" # tmp directory for storing data inside
MPOINT="./mount-point"

SRC="./src" #  Source file

FILE1="myFS.h"
FILE2="fuseLib.c"
 
if [ $MPOINT ]; then
	# Mount Directory exists. Removes files of the mount-point directory.
	rm -r $MPOINT/*
	echo "$MPOINT directory content deleted... [OK]"
	#rm -rf -- $TMP_DIRECTORY # INFO: http://stackoverflow.com/questions/820760/in-unix-how-do-you-remove-everything-in-the-current-directory-and-below-it
else
	mkdir $MPOINT # Create empty "MPOINT" directory
fi

if [ $TMP ]; then
	# Temp Directory exists. Removes files of the mount-point directory.
	rm -r $TMP/*
	echo "$TMP directory content deleted... [OK]"
	#rm -rf -- $TMP # INFO: http://stackoverflow.com/questions/820760/in-unix-how-do-you-remove-everything-in-the-current-directory-and-below-it
else
	mkdir $TMP # Create empty "temp" directory
fi

#(a) Copy two text files with size greater than 1 block (e.g., fuseLib.c and myFS.h) into the
#FS as well as into a temp directory (./temp for instance) in the Linux’s file system.

	# Copy file 1 into FileSystem and TMP
	cp $SRC/$FILE1 $TMP/$FILE1	 		# Copy into temporary directory
	cp $SRC/$FILE1 $MPOINT/$FILE1			# Copy into filesystem mount directory
	
	# Copy file 2 into FileSystem and TMP
	cp $SRC/$FILE2 $TMP/$FILE2			# Copy into temporary directory
	cp $SRC/$FILE2 $MPOINT/$FILE2			# Copy into filesystem mount directory

#(b) Check the integrity of the virtual disk with my-fsck and perform a diff between the
#original files and those copied to the FS. Truncate the first file (man truncate) in the
#temp folder and in your FS so as to reduce the file size in one block.

./my-fsck $MPOINT # Check integrity
 
DIFF=$(diff $TMP/$FILE1 $MPOINT/$FILE1) # http://unix.stackexchange.com/questions/33638/diff-several-files-true-if-all-not-equal

if [ "$DIFF" == "" ]; then
	echo "$TMP/$FILE1 and $MPOINT/$FILE1 are equals"
else
	echo "$TMP/$FILE1 and $MPOINT/$FILE1 are differents"
fi

#DIFFSECOND=$(diff c d)   

#if [ "$DIFFSECOND" != "" ]; then
#	echo "fuseLib.H and fuseLib.c are equals"
#else	
#	echo "fuseLib.H and fuseLib.c are different"
#fi

# truncate in one block

#truncatedFile=$MPOINT"/myFS.h"
#truncate(truncatedFile, 4096);
#echo $truncatedFile

#(c) Check the integrity of the virtual disk again and perform a diff between the original
#file and the truncated one.

#./my-fsck $MPOINT # Check integrity

#(d) Copy a third text file into your FS.

#cp ./$SRC/MyFileSystem.c $MPOINT/  # copy into m point

#(e) Check the integrity of the virtual disk and perform a diff between the original file
#and the one copied into the FS.

#./my-fsck $MPOINT

#(f) Truncate the second file in the temp folder and in your FS, so as to increase the file
#size in one block.


# ????????????????????????????????

#(g) Check the integrity of the disk and perform a diff between the original file and the
#truncated one.
