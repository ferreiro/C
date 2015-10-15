#!/bin/bash
if ! test -e mytar ; then
	echo "mytar file doesn't exist, or is not on the current directory"
	exit -1
elif ! test -x mytar ; then
	echo "mytar file is not executable"
	exit -1
else
	if test -d tmp ; then
		rm -r tmp
	fi
	mkdir tmp
	cd tmp
	file1=file1.txt
	touch $file1
	echo "Hello world!" > $file1
	file2=file2.txt
	touch $file2
	head /etc/passwd > $file2
	file3=file3.dat
	touch $file3
	head --bytes=1024 /dev/urandom > $file3
	../mytar -cf filetar.mtar $file1 $file2 $file3
	mkdir out
	cp ../mytar ./out
	cd out
	../../mytar -xf ../filetar.mtar
	if test -n "$(diff ../$file1 $file1 2>&1)" ; then
		echo "File $file1 is not exactly equal"
		exit -1
	elif test -n "$(diff ../$file2 $file2 2>&1)" ; then
		echo "File $file2 is not exactly equal"
		exit -1
	elif test -n "$(diff ../$file3 $file3 2>&1)" ; then
		echo "File $file3 is not exactly equal"
		exit -1
	else
		echo "Success"
		exit 0
	fi
fi

