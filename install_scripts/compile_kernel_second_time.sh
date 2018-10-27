make defconfig
make -j5
sudo make modules_install install
sudo update-grub2
