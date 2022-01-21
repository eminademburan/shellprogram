#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{

	int noOfCharacter = atoi(argv[1]);
	
	char randomCharacter;
	char string[noOfCharacter + 1];
	
	for( int i = 0; i < noOfCharacter; i++ )
	{
		scanf(" %c", &randomCharacter);
		string[i] = randomCharacter;
	}
	
	string[noOfCharacter] = '\0';
	
	printf("%s \n", string);
		
	return 0;
}

