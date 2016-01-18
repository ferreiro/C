#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define PATH "/dev/chardev_leds" // Path to write

int writeNumToFile(int filedesc, char *buf, int len) {
    int bytes_writed;
	if((bytes_writed = write(filedesc, buf, len)) < 0) {
		puts("Can not write to file!\n");
		return -1;
	}
	return 0;
}

int main() {
	
    int filedesc;
    char *buf;
    //int i = 0;
    
	if((filedesc = open(PATH, O_RDWR)) < 0) {
		puts("Hey!!! The file coudln't be opened");
		return 1;
	}
	if((writeNumToFile(filedesc, "23", 2)) == 0) {
		puts("COrrect!!!!");
	}
	if((writeNumToFile(filedesc, "123", 3)) == 0) {
		puts("COrrect!!!!");
	}
	/*
	while(i < 10) {
		
		buf = "1";
		
		
		sleep(2);
		
		buf = "2";
		if((bytes_writed = write(filedesc, buf, 1)) < 0) {
			puts("Can not write to file!\n");
			return 1;
		}
		
		buf = "3";
		sleep(2);
		if((bytes_writed = write(filedesc, buf, 1)) < 0) {
			puts("Can not write to file!\n");
			return 1;
		}
		puts("jojojoj");
		
		i++;
	} 
	*/
	
	if (close(filedesc) < 0) {
		puts("Can not close the file");
		return 1;
	} 
	/*
	puts ("writed!!!");
	buf = "";
	
	if((bytes_readed = read(filedesc, buf, 3)) < 0) {
		puts("Can not read from file!\n");
		return 1;
	}
	puts ("Readed!!!!!!");
	sleep(0.33);
	// Infinite while
	while (1==1) {
		
		
	}
	*/
	
	
	
	
	
	
	 
	
	/*
	nread=read(0,buffer,128);
    
    if(nread == -1)
     write(2,"an read error occured\n",26);
     
    if((write(1,buffer,nread))!= nread)
       write(2,"a write error occured\n",27);
	 */
	
	/*
    if(write(filedesc,"This will be output to testfile.txt\n", 36) != 36)
    {
        write(2,"There was an error writing to testfile.txt\n");    // strictly not an error, it is allowable for fewer characters than requested to be written.
        return 1;
    }
    * */
    
	
	return 0;
}
