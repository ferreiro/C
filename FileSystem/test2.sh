#!/bin/bash

MPOINT="./mount-point"

rm -R -f test
mkdir test

echo 'file 1' > ./test/file1.txt
echo "Copying file 1 into this directory- HOLAAAAAAAA. ESto es un archivo"
cp ./test/file1.txt $MPOINT/
#read -p "Press enter..."

echo "Creating file 2. THis file is somethig really really amazing....."
echo 'This is file 2' > $MPOINT/file2.txt
ls $MPOINT -la
#read -p "Press enter...

echo "Creating file 3"
echo 'This is file 3' > $MPOINT/file3.txt
ls $MPOINT -la 

echo "Creating file 3"
echo 'This is file 3' > $MPOINT/file3.txt
ls $MPOINT -la 

