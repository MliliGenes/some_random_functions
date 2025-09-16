#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void solve( int board[], int idx, int size );

int main( int ac, char** av) {
	
	if (ac != 2) return 1;

	int n = atoi(av[1]);
	int board[n];

	solve(board, 0, n);
}

bool safe( int board[], int row, int col, int size) {
	
	int offset = 1;

	for ( int i = row - 1; i >= 0; i --, offset++ ) {
		if (board[i] == col)
			return false;
		if (board[i] == col - offset  && col - offset >= 0)
			return false;
		if (board[i] == col + offset  && col + offset <= size)
			return false;
	}	
	
	return true;
}


void solve( int board[], int idx, int size ) {

	// breaking condition
	if (idx == size) {
		for (int c = 0; c < size; c++) {
			printf("%d", board[c]);
			if (c != size -1) printf(" ");
		}
		puts("");
		return;
	}
	
	for (int col = 0; col < size; col++) {
		board[idx] = col;
		if (safe(board, idx, col, size))
			solve(board, idx + 1, size);
	}
}
