Add the path of MinGW\bin, MinGW\msys\1.0\bin, and Python27 to environment variable "PATH"

For example, the path to be added should look like "C:\SoftwarePackage\MinGW\bin;C:\SoftwarePackage\MinGW\msys\1.0\bin;C:\SoftwarePackage\Python27" (without quotes)

In case you already had python or MinGW installed, the path to those directories should be removed from the environment

Open the file in4073\Makefile using any text editor and enter the COM port number of FCB under "upload" and "upload-run" command. For example, the code snippet after adding prefix looks like:

	python dfu_serial/./serial_dfu.py -p COM2 _build/in4073.bin
	
	where COM2 is the COM port of FCB

	Also, enter the correct COM port in pc_terminal\pc_terminal.c file
	
Open command prompt and naviage to in4073 folder. Type "make upload-run" and press enter. The code should compile and upload to the FCB. 
On successful compilation and upload of the program, the data sent from FCB should be displayed on command prompt at 115200 baud. (Any serial terminal program (example: Putty) can be used instead of provided pc_terminal program)

Note: Type "make upload" for uploading the program, and "make upload-run" for uploading and executing the terminal program.