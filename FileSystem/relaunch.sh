fusemount -u mount-points
rm-f /moutpoints/*
make
./fs-fuse -t 2097152 -a virtual-disk -f '-d -s mount-point'
