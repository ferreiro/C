#!/bin/bash

TMP="./temp" # tmp directory for storing data inside
MPOINT="./mount-point"

SRC="./src" #  Source file

FILE1="myFS.h"
FILE2="fuseLib.c"
FILE3="MyFileSystem.c"

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
		echo "[EQUALS] File $TMP/$FILE1 and $MPOINT/$FILE1 are equals"
	else
		echo "[Differents] File $TMP/$FILE1 and $MPOINT/$FILE1 are differents"
	fi


	DIFF=$(diff $TMP/$FILE2 $MPOINT/$FILE2) # http://unix.stackexchange.com/questions/33638/diff-several-files-true-if-all-not-equal

	if [ "$DIFF" == "" ]; then
		echo "[EQUALS] File $TMP/$FILE2 and $MPOINT/$FILE2 are equals"
	else
		echo "[Differents] File $TMP/$FILE2 and $MPOINT/$FILE2 are differents"
	fi

	# Truncate the first file (man truncate) in the temp folder and in your FS so as to reduce the file size in one block.

		# ¿?¿?¿?¿¿?¿?¿
		# truncate -s 0 {$TMP/$FILE1}
		# truncate("$TMP/$FILE1", 4096);
		# echo $truncatedFile

#(c) Check the integrity of the virtual disk again and perform a diff between the original
#file and the truncated one.

	./my-fsck $MPOINT # Check integrity
	 
	DIFF=$(diff $TMP/$FILE1 $MPOINT/$FILE1) # http://unix.stackexchange.com/questions/33638/diff-several-files-true-if-all-not-equal

	if [ "$DIFF" == "" ]; then
		echo "[EQUALS] Truncated File $TMP/$FILE2 and $MPOINT/$FILE2 are equals"
	else
		echo "[Differents] Truncated File $TMP/$FILE2 and $MPOINT/$FILE2 are differents"
	fi

#(d) Copy a third text file into your FS.

	cp $SRC/$FILE3 $MPOINT/$FILE3 # copy into m point
	echo "[COPIED] Third file into $MPOINT/MyFileSystem.c"

#(e) Check the integrity of the virtual disk and perform a diff between the original file
#and the one copied into the FS.

	./my-fsck $MPOINT # Check integrity
	DIFF=$(diff $SRC/$FILE3 $MPOINT/$FILE3) # http://unix.stackexchange.com/questions/33638/diff-several-files-true-if-all-not-equal

	if [ "$DIFF" == "" ]; then
		echo "[EQUALS] Third Copied file $SRC/$FILE3 and $MPOINT/$FILE3 are equals"
	else
		echo "[Differents] Truncated File $SRC/$FILE3 and $MPOINT/$FILE3 are differents"
	fi

#(f) Truncate the second file in the temp folder and in your FS, so as to increase the file
#size in one block.

	# ¿?¿?¿?¿¿?¿?¿ 
	# truncate("$TMP/$FILE2", 4096);
	# truncate("$MPOINT/$FILE2", 4096); 

#(g) Check the integrity of the disk and perform a diff between the original file and the
#truncated one.
	
	./my-fsck $MPOINT # Check integrity
	DIFF=$(diff $TMP/$FILE2 $MPOINT/$FILE2) # http://unix.stackexchange.com/questions/33638/diff-several-files-true-if-all-not-equal

	if [ "$DIFF" == "" ]; then
		echo "[EQUALS] Truncated Second file $TMP/$FILE2 and $MPOINT/$FILE2 are equals"
	else
		echo "[DIFFERENT] Truncated Second file $TMP/$FILE2 and $MPOINT/$FILE2 are DIFFERENT"
	fi
