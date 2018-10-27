sudo apt-get update && sudo apt-get upgrade
sudo apt-get install build-essential bin86 fakeroot kernel-package
sudo apt-get install libglade2-dev
sudo apt-get install libssl-dev

sudo adduser donato src

#a questo punto RIAVVIARE 

sudo chmod -R g+wr /usr/src

tar -xf linux-4.5.3.tar.xz
sudo mv linux-4.5.3_alberto /usr/src/
cd /usr/src/linux-4.5.3_alberto/

cp /boot/config-`uname -r`* .config 
make defconfig

sudo make-kpkg --rootcmd fakeroot --initrd kernel_image kernel_headers modules_image --append-to-version -alberto --jobs 4
#L’argomento di jobs {numero} rappresenta il numero di unità di lavoro CORE (eseguite contemporaneamente)

#Installare
cd ..

#sudo dpkg -i linux-image-[VERSIONE_DEL_KERNEL].deb
#sudo dpkg -i linux-headers-[VERSIONE_DEL_KERNEL].deb

sudo dpkg -i linux-image-4.5.3-alberto_4.5.3-alberto-10.00.Custom_amd64.deb
sudo dpkg -i linux-headers-4.5.3-alberto_4.5.3-alberto-10.00.Custom_amd64.deb

#Il boot loader GRUB verrà aggiornato automaticamente.

#Riavviare il computer e quindi scegliere il nuovo kernel installato.

#Se ci sono problemi, riavviare la macchina e utilizzare il vecchio kernel.
