#!/bin/bash

TMP_DIRECTORY=tmp # tmp directory for storing data inside
OUTPUT_DIRECTORY=out # tmp directory for storing data inside
MYTAR_FILE=mytar.mtar

FILENAME1=file1.txt
FILENAME2=file2.txt
FILENAME3=file3.dat

echo "---"
echo "Welcome to the tester"

# 1. Check that the mytar program is located in the current working directory and it is an actual executable file. If the check fails, print an error message to the standard output and exit.
# INFO | http://stackoverflow.com/questions/638975/how-do-i-tell-if-a-regular-file-does-not-exist-in-bash

if [ ! -e ./mytar ]; then
    echo "[ ERROR ] Mytar File not found!"
    exit -1 # exit the program with a fail
    # return 1
elif [ ! -x ./mytar ]; then
	# Check that mytar is an executable file
	echo "[ ERROR ] Mytar is not executable"
    exit -1 # exit the program with a fail
fi

echo "Mytar was found and is executable [OK]" # At this point mytar is executable and exits
echo "---"

# 2. Check if a directory called "tmp" is found in the current working directory. If so, remove the directory and its contents recursively (check out documentation on the -r option of the rm command).

if [ -d "$TMP_DIRECTORY" ]; then
	# Removes Directory if the folder exists
	rm -rf -- $TMP_DIRECTORY  # INFO: http://stackoverflow.com/questions/820760/in-unix-how-do-you-remove-everything-in-the-current-directory-and-below-it
	echo "$TMP_DIRECTORY directory deleted... [OK]"
fi

# 3. Create a temporary directory named tmp in the current working directory. The script will then switch to that directory with cd.

mkdir $TMP_DIRECTORY # Create "tmp" empty directory
cd $TMP_DIRECTORY	  # move to tmp directory

# 4. The following three files will be created inside tmp:
# Create 3 text files in the tmp folder
# INFO: http://stackoverflow.com/questions/4662938/create-text-file-and-fill-it-using-bash

if [ ! -e $FILENAME1 ]; then
	touch $FILENAME1 # Create a hello World file
	echo "Hello World!" > $FILENAME1
  	echo "$FILENAME1 created... [OK]"
fi
if [ ! -e $FILENAME2 ]; then
	touch $FILENAME2 # Copy 10 lines from /etc/passwd file
	head -10 /etc/passwd > $FILENAME2 # INFO: http://stackoverflow.com/questions/1325701/how-to-get-the-copy-the-first-few-lines-of-a-giant-file-and-add-a-line-of-text-a
  	echo "$FILENAME2 created... [OK]"
fi
if [ ! -e $FILENAME3 ]; then
	touch $FILENAME3 # Create a .bat file with /dev/urandom generator
	head -c 1024 /dev/urandom > $FILENAME3
  	echo "$FILENAME3 created... [OK]"
	# head --bytes=1024 /dev/urandom > $FILENAME3
fi

# 5. Invoke the mytar program so as to create a filetar.mtar archive including the contents of the three aforementioned files.

./../mytar -c -f $MYTAR_FILE $FILENAME1 $FILENAME2 $FILENAME3

# 6. Create the out directory within the current working directory (i.e., tmp). Create a copy of the filetar.mtar file in out.

if [ ! -d $OUTPUT_DIRECTORY ]; then
	mkdir $OUTPUT_DIRECTORY # create out directory
	echo "Creating a new out directory [OK]"
fi

cp ./$MYTAR_FILE ./$OUTPUT_DIRECTORY/$MYTAR_FILE # copy mytar to out/mytar

# 7. Switch to the out directory and run mytar to extract the tarball’s contents.

cd $OUTPUT_DIRECTORY
./../../mytar -x -f $MYTAR_FILE # extract files in the out folder

# 8. Use the diff program to compare the contents of each extracted file with the associated original file found in the parent directory (..).
# INFO: http://www.linuxquestions.org/questions/linux-newbie-8/bash-script-to-compare-two-files-563836/

if diff ../$FILENAME1 $FILENAME1 >/dev/null ; then
  	echo "$FILENAME1 are equal in both directories [OK]"
else
  	echo "$FILENAME1 are NOT equal in both directories [ ERROR | EXIT]"
  	exit -1
fi

if diff ../$FILENAME2 $FILENAME2 >/dev/null ; then
  	echo "$FILENAME2 are equal in both directories [OK]"
else
  	echo "$FILENAME2 are NOT equal in both directories [ ERROR | EXIT ]"
  	exit -1
fi

if diff ../$FILENAME3 $FILENAME3 >/dev/null ; then
  	echo "$FILENAME3 are equal in both directories [OK]"
else
  	echo "$FILENAME3 are NOT equal in both directories [ ERROR | EXIT ]"
	exit -1
fi

echo "-- Finish tests --"
echo "[Hoooooooray!]: All tests were executed correctly"
exit 0 # at this point all the tests were passed. So, we have success!!
