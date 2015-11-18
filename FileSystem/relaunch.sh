fusermount -u mount-point
make
cd mount-point/
rm *
cd ../
./fs-fuse -t 2097152 -a virtual-disk -f '-s -d mount-point'
