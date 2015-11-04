#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	char *s1 = malloc(5);

	printf("%lu\n", strlen(s1));
	strcpy(s1, "tutorialspoint");
	printf("%lu\n", strlen(s1));

	printf("%s\n" , s1);

	// strcmp(const char *s1, const char *s2);

	return 0;
}



