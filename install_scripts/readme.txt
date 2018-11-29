INSTRUCTIONS TO DOWNLOAD, COMPILE AND INSTALL THE NEW KERNEL AND PAPI LIBRARY

0. Start with Ubuntu 16.04.1, that should have kernel 4.4.0
	uname -r

1. Download the kernel:
	https://www.kernel.org/pub/linux/kernel/v4.x/
	[Select linux-4.5.3.tar.gz]

2. Extract the compressed file
	tar -xvzf linux-4.5.3.tar.gz

3. Copy and paste the modified files before compiling
	look at "Kernel-4_5_3/modified_kernel_source" folder: each folder name corresponds to the path for source code

4. Go to "linux-4.5.3" folder and run "compile_kernel_first_time.sh"

5. Reboot the system. Now the kernel should be changed
	uname -r

6. Download PAPI library:
	http://icl.cs.utk.edu/papi/software/index.html
	[Select papi-5.5.1.tar.gz]

7. Create a new empty folder where PAPI library will be installed [MYPATH]

8. Change the paths inside the script "install_papi.sh", using MYPATH created at point 7.

9. Go to the folder that contains the compressed PAPI library and run "install_papi.sh"

10. To use PAPI library, compile a program:
	gcc filename.c -lpapi -I MYPATH/include -L MYPATH/lib

11. To run a program on a single core:
	taskset -c 1 ./a.out
