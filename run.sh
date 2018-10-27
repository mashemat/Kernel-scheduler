matrix_size=(10000) 
for sz in `seq 0` ; do 
echo "matrix size:" ${matrix_size[sz]}
taskset -c 3 ./pm_v.bin ${matrix_size[sz]}
sync; echo 3 > /proc/sys/vm/drop_caches
sync; echo 2 > /proc/sys/vm/drop_caches
sync; echo 1 > /proc/sys/vm/drop_caches
echo @*********@
done
