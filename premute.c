#include <stdio.h>

void sort(char* str);

int premute( char* str );
	
int main(int ac, char **av) {
	if (ac != 2) return 1;
	char* str = av[1];

	sort(str);
	puts(str);
	while (premute(str))
		puts(str);
}

void sort(char* str) {
	for (int i = 0; str[i]; i++) {
	   for (int j = i; str[j]; j++) {
			if ( str[i] > str[j] )
			{
				char tmp = str[i];
				str[i] = str[j];
				str[j] = tmp;
			}
	   }
	}
}

int _strlen( char* str ) {
	if (!str) return 0;
	int i = 0;
	for (i = 0; str[i]; i++)
		continue;
	return i;
}

int premute( char* str ) {
	int target = _strlen(str) - 1;

	while (target > 0 && str[target - 1] > str[target])
		target --;

	if (target <= 0)
		return 0;

	int swap = _strlen(str) - 1;

	while ( swap > 0 && str[swap] < str[target - 1] )
		swap --;

	char tmp = str[swap];
	str[swap] = str[target -1];
	str[target - 1] = tmp;

	sort(str + target);
	return 1;
}
