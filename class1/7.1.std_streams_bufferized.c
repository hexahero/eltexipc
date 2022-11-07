#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
int main(int argc, char *argv[]) {
	
    FILE* file = fopen(argv[1], "w");
    char inputbuffer[256];
	
	if (file == NULL)
        printf("failed to open file");
	
	printf(">");
	scanf("%s", inputbuffer);
	
	fprintf(file, "%s\n", inputbuffer);

    fclose(file);
	
    return 0;
}