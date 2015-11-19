fusermount -u mount-point
make 
rm -r ./mount-point/*

if [ -d "./mount-point/" ]; then
	# Mount Directory exists. Removes files of the mount-point directory.
	rm -r $MPOINT/*
	echo "$MPOINT directory content deleted... [OK]"
	#rm -rf -- $TMP_DIRECTORY # INFO: http://stackoverflow.com/questions/820760/in-unix-how-do-you-remove-everything-in-the-current-directory-and-below-it
else
	mkdir $MPOINT # Create empty "MPOINT" directory
fi

./fs-fuse -t 2097152 -a virtual-disk -f '-s -d mount-point'
