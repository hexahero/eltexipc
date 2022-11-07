#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

char* appPath;
char** arguments;
unsigned int numOfArgs;

void exec() {
	
	// Setup
	appPath = (char*)malloc(PATH_MAX);
	
	printf("application to execute>");
	scanf("%s", appPath);
	printf("enter num of arguments>");
	scanf("%d", &numOfArgs);
	numOfArgs += 2;
	
	arguments = (char**)malloc(numOfArgs);
	
	for (int i = 0; i < numOfArgs - 1; i++)
		arguments[i] = (char*)malloc(32);
	
	arguments[0] = appPath;
	
	for(int i = 1; i < numOfArgs - 1; i++) {
		
		printf("enter argument (%d)>", i);
		scanf("%s", arguments[i]);
		
	}
	
	arguments[numOfArgs - 1] = NULL;

	// Run app
	execv(appPath, arguments);
	
	// Cleanup
	for (int i = 0; i < numOfArgs - 1; i++)
		free(arguments[i]);
	
	free(arguments);
	free(appPath);
	
}

int main() {
    
    exec();
    
	return 0;
}