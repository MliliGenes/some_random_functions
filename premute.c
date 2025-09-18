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

int _strlen( char* str ) {
	if (!str) return 0;
	int i = 0;
	for (i = 0; str[i]; i++)
		continue;
	return i;
}

void sort(char* str) {
	int len = _strlen(str);
	int j = 0;
	while ( j < len )
	{
        int i = 0;
        while ( i < len - j ) {
            if ( i + 1 < len && str[i] > str[i+ 1] ) {
                char tmp = str[i + 1];
                str[i + 1] = str[i];
                str[i] = tmp;
            }
            i++;
        }
		j++;
	}
}

int premute( char* str ) {
	int target = _strlen(str) - 1;

	while (target > 0 && str[target - 1] >= str[target])
		target --;

	if (target <= 0)
		return 0;

	int swap = _strlen(str) - 1;

	while ( swap > 0 && str[swap] <= str[target - 1] )
		swap --;

	char tmp = str[swap];
	str[swap] = str[target -1];
    str[target - 1] = tmp;

	sort(str + target);
	return 1;
}
