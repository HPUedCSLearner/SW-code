bsub -I  -q q_share -b -n 1 -cgsp 64 -host_stack 1024 -share_size 6500 ./build/a.out
