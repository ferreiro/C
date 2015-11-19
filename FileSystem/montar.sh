MPOINT="./mount-point"

rm -r $MPOINT/*
fusermount -u mount-point
make 
./fs-fuse -t 2097152 -a virtual-disk -f '-s -d mount-point'
