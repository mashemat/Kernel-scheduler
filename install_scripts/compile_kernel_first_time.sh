make mrproper
sudo apt-get install libncurses5-dev gcc make git exuberant-ctags bc libssl-dev
sudo apt-get update && sudo apt-get install cvs cmake-curses-gui
cp /boot/config-`uname -r`* .config
make defconfig
make -j5
sudo make modules_install install
sudo update-grub2
