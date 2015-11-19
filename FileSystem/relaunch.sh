MPOINT="./mount-point"

fusermount -u mount-point
rmdir $MPOINT/
mkdir $MPOINT/
make 
#./fs-fuse -t 2097152 -a virtual-disk -f '-s -d mount-point'
./fs-fuse -a virtual-disk -m -f '-d -s mount-point'
