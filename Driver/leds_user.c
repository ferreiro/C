#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

static int fileDescriptor;
#define PATH "/dev/chardev_leds" // Path to write

#define N 3
#define NUMLOCK "1"
#define CAPSLOCK "2"
#define SCROLLLOCK "3"
#define SLEEP_TIME 2 // number in seconds

static int led[3] = { NUMLOCK, CAPSLOCK, SCROLLLOCK };

int menu();
int setLed(int filedesc, char *buf, int len);
void fountain();

int main() {
	bool finish= false;
	int i = 0, option;

	printf("Welcome to my awesome program!!");

	if((fileDescriptor = open(PATH, O_RDWR)) < 0) {
		puts("Hey!!! The file coudln't be opened");
		return 1;
	}

	while (!finish) {

		option = menu();

		if (option==0) // exit program
			finish=true;
		else if (option == 1) // fountain
			fountain();
		else if (option == 2) // virus
			virus();
		else if (option == 3) // cars
			cars();
		else if (option == 4) // random
			random();
	}

	if (close(filedesc) < 0) {
		puts("Can not close the file");
		return 1;
	} 

	/*

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

int menu() {
	int option = -1;
	bool valid = false;

	puts(" Option | Description\n");
	puts("   1    | Las vegas Fountain\n");
	puts("   2    | Crazy Fountain\n");
	puts("   3    | Fast and Furious - Speed racing\n");
	puts("   4    | RVndom\n"); // Chooses one of the previous modes randomly
	puts("   0    | Exit\n");

	while (!valid) {
		puts("Which option do you want? (type the number): \n");
		scanf("%d", &option);
		if (option >= 0 && option <= 4)
			valid=true
		else
			puts("Wrong option! Try again\n");
	}

	return option;
}

int setLed(int filedesc, char *buf, int len) {
    int bytes_writed;
	if((bytes_writed = write(filedesc, buf, len)) < 0) {
		puts("Can not write to file!\n");
		return -1;
	}
	return 0;
}

void fountain() {
	int i=0, total = 3;
	//char *buf;

	for (i=0; i < total; i++) {
		// Go right
		for (j=0; j < N; j++) { 
			if((setLeft(fileDescriptor, led[j], 1)) != 0) {
				puts(led[j]);
				puts("Problems setting the ledt");
			}
			sleep(SLEEP_TIME);
		}
		// Go left
		for (j=N; j > 0; j--) {
			if((setLeft(fileDescriptor, led[j], 1)) != 0) {
				puts(led[j]);
				puts("Problems setting the ledt");
			}
			sleep(SLEEP_TIME);
		}
	}
}











/*
	int i = 0;
	char *buf;

	for (i=0; i < 3; i++) {
		setLed(int filedesc, char *buf, int len)
		buf = 1;
		if((setLeft(filedesc, "23", 2)) == 0) {
			puts("[X] Correct");
		}
	}
*/