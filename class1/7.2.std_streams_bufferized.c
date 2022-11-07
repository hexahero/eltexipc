#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
 
int main(int argc, char *argv[]) {
	
    FILE* file = fopen(argv[1], "r");
	char readbuff[2];
	
    if (file == NULL)
        printf("failed to open file");
 
    while (true) {
	
		int result = fread(readbuff, 1, 1, file);
		
		if (result == -1 || result == 0)
			break;
		
		printf("%s", readbuff);
		
    }

    fclose(file);
	
    return 0;
}