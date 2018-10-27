#before running this, create a folder at MYPATH and change the script
#open the terminal in the folder where there is papi download compressed
tar -xvf papi-5.5.1.tar.gz
cd papi-5.5.1/src
./configure --prefix=/home/alberto/Desktop/papi #change with MYPATH
make
make install

#remove and compile the libraries
cd /home/alberto/Desktop/papi/lib #change with MYPATH/lib
rm libpapi.so
rm libpapi.so.5
ln -s libpapi.so
ln -s libpapi.so.5

