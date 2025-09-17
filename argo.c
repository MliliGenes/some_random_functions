#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>:

typedef enum {
	MAP,
	INTEGER,
	STRING
} e_type;

typedef struct map {
	struct pair	*data;
	size_t		size;
} t_map;

typedef struct	json {
	e_type type;
	union {
		t_map map;
		int	integer;
		char	*string;
	};
}	json;

typedef struct	pair {
	char	*key;
	json	value;
}	pair;

int	peek(FILE *stream)
{
	int	c = getc(stream);
	ungetc(c, stream);
	return c;
}

void	unexpected(FILE *stream)
{
	if (peek(stream) != EOF)
		printf("unexpected token '%c'\n", peek(stream));
	else
		printf("unexpected end of input\n");
}

int	accept(FILE *stream, char c)
{
	if (peek(stream) == c)
	{
		(void)getc(stream);
		return 1;
	}
	return 0;
}

int	expect(FILE *stream, char c)
{
	if (accept(stream, c))
		return 1;
	unexpected(stream);
	return 0;
}

void	free_json(json j)
{
	switch (j.type)
	{
		case MAP:
			for (size_t i = 0; i < j.map.size; i++)
			{
				free(j.map.data[i].key);
				free_json(j.map.data[i].value);
			}
			free(j.map.data);
			break ;
		case STRING:
			free(j.string);
			break ;
		default:
			break ;
	}
}

void	serialize(json j)
{
	switch (j.type)
	{
		case INTEGER:
			printf("%d", j.integer);
			break ;
		case STRING:
			putchar('"');
			for (int i = 0; j.string[i]; i++)
			{
				if (j.string[i] == '\\' || j.string[i] == '"')
					putchar('\\');
				putchar(j.string[i]);
			}
			putchar('"');
			break ;
		case MAP:
			putchar('{');
			for (size_t i = 0; i < j.map.size; i++)
			{
				if (i != 0)
					putchar(',');
				serialize((json){.type = STRING, .string = j.map.data[i].key});
				putchar(':');
				serialize(j.map.data[i].value);
			}
			putchar('}');
			break ;
	}
}

int	argo(json *dst, FILE *stream);

int main ( int ac, char** av ) {
	if ( ac != 2 ) return 1;
	FILE *stream = fopen(av[1], "r");
	
	json *head = NULL;
}

e_type get_type( char c ) 
{
	if (c == '\"') return STRING;
	if (isdigit(c) || c == '-') return INTEGER;
	if (c == '{') return MAP;
	return -1;
}

json *create_node( int type, void *data ) {
	json *node = malloc( sizeof(json) );
	if (!node) return NULL;
	node->type = type;
	switch ( type ) {
		case STRING:
			node->string = data;
			break;
		case INTEGER:
			node->integer = *(int*)data;
			break;
		case MAP:
			node->map = *(t_map *)data;
			break;
	}
	return node;
}

json* extract_str ( FILE* s ) {
	
	if (!accept(s, '\"'))
		return NULL;

	int cap = 16;
	int len = 0;
	char *str = malloc(cap);
	if (!str) return NULL;

	while ( true ) {
		if (len >= cap) {
			cap *= 2;
			char *new_str = realloc(str, cap);
			if (!new_str) {
				free(str);
				return NULL;
			}
			str = new_str;
		}

		char le_peek = peek(s);
		if (le_peek == '\"') {
			str[len] = '\0';
			getc(s);
			break;
		}
		if (le_peek == EOF) {
			free(str);
			return NULL;
		}
		if (le_peek == '\\') {
			getc(s);
		}

		str[len++] = getc(s);
	}

	return create_node( STRING, str );

}

json *extract_int ( FILE* s ) {
	int sign = 1;
	int nbr = 0;

	if ( peek(s) == '-' ) {
		sign *= -1;
		getc( s );
	}

	while (isdigit(peek(s))) {
		nbr = nbr * 10 + getc(s) - '0'; 
	}

	return nbr * sign;
}


json *parse ( json* head, FILE* stream ) {

	char le_peek = peek( stream );

	e_type le_type = get_type ( le_peek );
	if  ( le_type == -1 )
		return NULL;

	if ( le_type == STRING )
		return extract_str( stream );
	if ( le_type == INTEGER )

	if ( le_type == MAP)

	return NULL;
}

int argo(json* dst, FILE* stream) {
	




}
