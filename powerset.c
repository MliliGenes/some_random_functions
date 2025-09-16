#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	int *subset;
	int size;
	int total;
} t_subset;

void powerset(int *set, int idx, int size, int total, t_subset obj);

int main( int ac, char **av ) {
	if (ac < 3) return 1;

	int total = atoi(av[1]);

	int *set = malloc((ac - 2) * sizeof(int));
	for (int i = 2; av[i]; i++)
		set[i-2] = atoi(av[i]);
	
	int *subset = malloc((ac-2) * sizeof(int));
	t_subset obj = {subset, 0, 0};

	powerset(set, 0, ac - 2, total, obj);
}

void powerset(int *set, int idx, int size, int total, t_subset obj) {
	
	if (obj.total == total) {
		// printing the subset
		for ( int i = 0; i < obj.size; i++) {
			printf("%d", obj.subset[i]);
			if ( i != obj.size -1 ) printf(" ");
		}
		puts("");
		return ;
	}
	
	for (int start = idx; start < size; start++) {
		obj.subset[obj.size] = set[start];
		powerset(set, start + 1, size, total,
				(t_subset){
					obj.subset, obj.size + 1, obj.total + set[start]
				});
	}

}
