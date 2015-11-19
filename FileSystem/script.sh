#!/bin/bash

TMP_DIRECTORY="./temp" # tmp directory for storing data inside
MPOINT="./mount-point"
SRC="./src"
 
if [ $MPOINT ]; then
	# Mount Directory exists. Removes files of the mount-point directory.
	rm -r $MPOINT/*
	echo "$MPOINT directory content deleted... [OK]"
	#rm -rf -- $TMP_DIRECTORY # INFO: http://stackoverflow.com/questions/820760/in-unix-how-do-you-remove-everything-in-the-current-directory-and-below-it
else
	mkdir $MPOINT # Create empty "MPOINT" directory
fi

if [ $TMP_DIRECTORY ]; then
	# Temp Directory exists. Removes files of the mount-point directory.
	rm -r $TMP_DIRECTORY/*
	echo "$TMP_DIRECTORY directory content deleted... [OK]"
	#rm -rf -- $TMP_DIRECTORY # INFO: http://stackoverflow.com/questions/820760/in-unix-how-do-you-remove-everything-in-the-current-directory-and-below-it
else
	mkdir $TMP_DIRECTORY # Create empty "temp" directory
fi

#(a) Copy two text files with size greater than 1 block (e.g., fuseLib.c and myFS.h) into the
#FS as well as into a temp directory (./temp for instance) in the Linux’s file system.

cp ./src/myFS.h $MPOINT/  			# copy into mount-point directory
cp ./src/fuseLib.c $MPOINT/			# copy into directory
cp ./src/myFS.h $TMP_DIRECTORY/  	# copy into tmp directory.
cp ./src/fuseLib.c $TMP_DIRECTORY/  # copy into temp directory.

#(b) Check the integrity of the virtual disk with my-fsck and perform a diff between the
#original files and those copied to the FS. Truncate the first file (man truncate) in the
#temp folder and in your FS so as to reduce the file size in one block.

# Check integrity
./my-fsck $MPOINT

$MPOINT/myFS.h > a
$SRC/myFS.h > b

DIFFFIRST=$(diff a b)   # http://stackoverflow.com/questions/3611846/bash-using-the-result-of-a-diff-in-a-if-statement

if [ "$DIFFFIRST" != "" ]; then
	echo "MyFS.H and myFS.c are equals"
else
	echo "MyFS.H and myFS.c are different"
fi

$MPOINT/fuseLib.c > c
$SRC/fuseLib.c > d

DIFFSECOND=$(diff c d)   

if [ "$DIFFSECOND" != "" ]; then
	echo "fuseLib.H and fuseLib.c are equals"
else	
	echo "fuseLib.H and fuseLib.c are different"
fi

# truncate in one block

truncatedFile=$MPOINT"/myFS.h"
echo $truncatedFile

#truncate("./"+$MPOINT+"/myFS.h", 4096);
#truncate("./"+$TMP_DIRECTORY+"/myFS.h", 4096);

#(c) Check the integrity of the virtual disk again and perform a diff between the original
#file and the truncated one.

#(d) Copy a third text file into your FS.

#cp ./$SRC/MyFileSystem.c $MPOINT/  # copy into m point

#(e) Check the integrity of the virtual disk and perform a diff between the original file
#and the one copied into the FS.

#./my-fsck $MPOINT

#(f) Truncate the second file in the temp folder and in your FS, so as to increase the file
#size in one block.

#(g) Check the integrity of the disk and perform a diff between the original file and the
#truncated one.
