#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>  

static int fileDescriptor;

#define SLEEP_TIME 130 // number in seconds
#define PATH "/dev/chardev_leds" // Path to write
 
int menu();
int setLed(int filedesc, char *buf);
void fountain();
void randomPattern();
void stroves();
void binaryCounter();

void SleepMs(int ms) {
usleep(ms*1000); //convert to microseconds
return;
}

int main() {
	bool finish= false;
	int option;

	printf("\nWelcome to my awesome program!!");

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
		else if (option == 2) // random
			randomPattern();
		else if (option == 3) // stroves
			stroves(); 
		else if (option == 4) // binary counter
			binaryCounter();			
			
		if((setLed(fileDescriptor, "")) != 0) {
			puts("Problems setting the ledt");
		}
	}

	if (close(fileDescriptor) < 0) {
		puts("Can not close the file");
		return 1;
	} 

	return 0;
}

int menu() {
	int option = -1;
	bool valid = false;

	puts("\n\n Option | Description");
	puts("   1    | Las vegas Fountain");
	puts("   2    | RVndom Stroves"); // Chooses one of the previous modes randomly
	puts("   3    | Fast and Furious Speed Racing");
	puts("   4    | Crazy Binary Lights Counter");
	puts("   0    | Exit\n");

	while (!valid) {
		puts("Which option do you want? (type the number): \n");
		scanf("%d", &option);
		if (option >= 0 && option <= 4)
			valid=true;
		else
			puts("Wrong option! Try again\n");
	}

	return option;
}

int setLed(int filedesc, char *buf) {
	int len = strlen(buf);
    int bytes_writed;
	if((bytes_writed = write(filedesc, buf, len)) < 0) {
		puts("Can not write to file!\n");
		return -1;
	}
	return 0;
}

void fountain() {
	int i=0, j=0, total = 10;
	char *ledsNumber = "123";
	char c;
 
	for (i=0; i < total; i++) {
		for (j = 0; j < 4; j++) {
			c = ledsNumber[j];
			if (j == 3) {
				c = ledsNumber[1];
			} 
			if((setLed(fileDescriptor, &c)) != 0) {
				puts("Problems setting the ledt");
			}
			SleepMs(170);
		}
	}
}

void randomPattern() {
	int i, n, num;
	time_t t;
	char *buff;
	char tmpBuff[10]; 

	n = 60;
	srand((unsigned) time(&t));

	puts("Executing Random");
	
	/* Print 5 random numbers from 0 to 49 */
	for( i = 0 ; i < n ; i++ ) {
		
		num = (random() % 3) + 1;
		sprintf(tmpBuff, "%d", num);
		buff = tmpBuff;
		
		if((setLed(fileDescriptor, buff)) != 0) {
			puts("Problems setting the ledt");
		}
		
		SleepMs(SLEEP_TIME);
	}
	
	puts("FInished");
   
}

void stroves() {
	int n = 10000, i;
	char *buff;
	int time = 3000;
 
	for( i = 0 ; i < n ; i++ ) {
		
		if (i % 2 == 0) {
			buff = "13";
		}
		else {
			buff = "22";
		}
		
		puts(buff);
	 
		if((setLed(fileDescriptor, buff)) != 0) {
			puts("Problems setting the ledt");
		}
		
		time /= 1.25;
		SleepMs( time );
	}
	
	puts("FInished");
}


void binaryCounter() {
	char* buff;
	int i;
	
	for (i=0; i <= 7; i++) {
		
		if (i==0) buff = "0";
		else if (i==1) buff = "3"; 
		else if (i==2) buff = "2";
		else if (i==3) buff = "23";
		else if (i==4) buff = "1";
		else if (i==5) buff = "13";
		else if (i==6) buff = "12";
		else if (i==7) buff = "123";
		 
		puts(buff);
		
		if((setLed(fileDescriptor, buff)) != 0) {
			puts("Problems setting the ledt");
		}
		
		SleepMs(1000);
		
	}
}
