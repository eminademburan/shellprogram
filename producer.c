#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{

	int noOfCharacter = atoi(argv[1]);
	
	char randomCharacter;
	
	for( int i = 0; i < noOfCharacter; i++ )
	{
		randomCharacter = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqqrstuvwxzyz0123456789"[random () % 62];
		printf("%c", randomCharacter);
	}
		
	return 0;
}

