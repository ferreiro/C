// Page 3 of "assigment2" pdf.
// Implement a program that copies the contents of 
// a file into another one (see Fig- ure 2.1).

#include <stdio.h>
#include <stdlib.h>

int copy(char *readfileName, char *writefileName);

int main() {
	char readfileName[] = "origin.txt";
	char writefileName[] = "destiny.txt";

	/*
	printf("String direction: %p\n", &readfileName);
	printf("String direction: %p\n", readfileName);
	printf("String text: %s\n", readfileName);
	*/

	if(copy(readfileName, writefileName)) {
		printf("%s\n", "Copied");
	}

	return 0;
}

int copy(char *readfileName, char *writefileName) {

	int opened = open(readfileName, O_CREAT , "a");

	printf("%s\n", readfileName);
	return -1;
}