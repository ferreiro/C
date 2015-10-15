# Mytar program
What's Mytar? Is a simple program to "compress" and extract a file. The basic idea is that you can create a .mtar file or parse a given mtar (so the programm will create the .txt files with the data from mtar).

### How to run the program?

1. For executing the Mytar program you just only need to:
	* Download/fork this repository. 
	* Open in the terminal the directory where this project is on your computer
	* Type on the console: make (* you can type make clean in order to clean previous .o files)
	* Program's built in commands: ./mytar -c|-x -f myTarFile.mtar [file1, file2, file3]  (before running this command, you have to execute: make on your terminal!)
		1. If you want to compress 3 files, type in the console: ./mytar -c yourTarFileDesiredName.mtar file1.txt file2.txt file3.txt
		2. To extraxt a .mtar file, type: ./mytar -x -f mytarFileName.mtar
		
2. Questions? Ask me via DM or open an issue on github ;-)

### Tests / Bash script

If you want to make a test. You can run the bash script i made.
In your terminal (be on the directoy for the project) type this command: ./test.sh (make sure the bash file is in executable mode, if not. type: chmod +x test.sh) 
