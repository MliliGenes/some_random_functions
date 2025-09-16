#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int open;  
    int close;
} Unbalanced;

Unbalanced count_extra ( char * );
bool valid ( char*  );
void rip ( char *, int , Unbalanced );

int main(int ac, char** av) {
	if (ac != 2) return 1;

	char *str = av[1];
	Unbalanced balance = count_extra(str);
	if (valid(str)) {
		puts(str);
		return 0;
	}

	rip(str, 0, balance);

	return 0;
}

void rip ( char *str, int idx, Unbalanced balance ) {
	if (str[idx] == 0) {
		if (valid(str) && balance.open == 0 && balance.close == 0)
			puts(str);
		return;
	}

	if ((str[idx] == '(' && balance.open > 0)
			|| (str[idx] == ')' && balance.close > 0)) {

		char swap = str[idx];
		str[idx] = ' ';
		Unbalanced new_balance = {
			(swap == '(' ? balance.open - 1: balance.open),
			(swap == ')' ? balance.close - 1: balance.close)
		};
		rip(
			str,
			idx + 1,
			new_balance
		);
		str[idx] = swap;
	}
	
	rip( str, idx + 1, balance );
}

bool valid ( char* str ) {
	
	int checker = 0;

	for (int i = 0; str[i]; i++) {
		char s = str[i];
		if ( s == '(' ) checker++;
		else if ( s == ')' ) {
			checker --;
			if ( checker < 0 ) return false;
		}
	}
	if ( checker == 0 ) return true;
	return false;
}

Unbalanced count_extra(char *str) {

	int o = 0, c = 0;

	for (int i = 0; str[i]; i++) {
		char s = str[i];
		if ( s == '(') o++;
		if ( s == ')') {
			if (o > 0) o--;
			else c++;
		}
	}
	
	Unbalanced res = {o, c};
	return res;
}
