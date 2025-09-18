#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

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
	if ( !stream ) return 1;
	json head;
	if ( argo( &head, stream ) == -1 ) {
		fclose(stream);
		return 1;
	}
	serialize(head);
	puts("");
	free_json(head);
	fclose(stream);
	return 0;
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

bool extract_str(json *dst, FILE *s) {
    if (!accept(s, '"')) { unexpected(s); return false; }

    int cap = 16, len = 0;
    char *str = malloc(cap);
    if (!str) return false;

    while (true) {
        if (len >= cap) {
            cap *= 2;
            char *tmp = realloc(str, cap);
            if (!tmp) { free(str); return false; }
            str = tmp;
        }

        char c = peek(s);
        if (c == '"') { str[len] = '\0'; getc(s); break; }
        if (c == EOF) { free(str); 	unexpected(s); return false; }
        if (c == '\\') getc(s);

        str[len++] = getc(s);
    }

    dst->type = STRING;
    dst->string = str;
    return true;
}


bool extract_int(json *dst, FILE *s) {
    int sign = 1;
    long nbr = 0;

    if (peek(s) == '-') {
        sign = -1;
        getc(s);
    }

    if (!isdigit(peek(s))) {
        unexpected(s);
        return false;
    }

    while (isdigit(peek(s))) {
        nbr = nbr * 10 + (getc(s) - '0');
    }

    dst->type = INTEGER;
    dst->integer = (int)(nbr * sign); // truncate to int
    return true;
}



bool parse ( json* head, FILE* stream );

bool extract_map(json *dst, FILE *s) {
    if (!accept(s, '{')) {
        unexpected(s);
        return false;
    }

    int cap = 5;
    int len = 0;
    pair *data = malloc(sizeof(pair) * cap);
    if (!data) return false;

    while (true) {
        if (len >= cap) {
            cap *= 2;
            pair *new_data = realloc(data, sizeof(pair) * cap);
            if (!new_data) {
                for (int idx = 0; idx < len; idx++) {
                    free(data[idx].key);
                    free_json(data[idx].value);
                }
                free(data);
                return false;
            }
            data = new_data;
        }

        char c = peek(s);
        if (c != '"') {
            free(data);
            unexpected(s);
            return false;
        }

        json key;
        if (!extract_str(&key, s)) {
            free(data);
            return false;
        }

        if (!expect(s, ':')) {
            free(data);
            free(key.string);
			unexpected(s);
            return false;
        }

        json value;
        if (!parse(&value, s)) {
			for (int idx = 0; idx < len; idx++) {
				free(data[idx].key);
				free_json(data[idx].value);
			}
            free(data);
            free(key.string);
            return false;
        }

        data[len++] = (pair){ .key = key.string, .value = value };

        c = peek(s);
        if (c == ',') {
            accept(s, ',');
            continue;
        }
        break;
    }

    if (!expect(s, '}')) {
        for (int idx = 0; idx < len; idx++) {
            free(data[idx].key);
            free_json(data[idx].value);
        }
        free(data);
		unexpected(s);
        return false;
    }

    dst->type = MAP;
    dst->map.data = data;
    dst->map.size = len;
    return true;
}

bool parse ( json* head, FILE* stream ) {

	char le_peek = peek( stream );

	e_type le_type = get_type ( le_peek );
	if  ( le_type == -1 )
		return false;

	if ( le_type == STRING )
		return extract_str( head, stream );
	if ( le_type == INTEGER )
		return extract_int( head, stream );
	if ( le_type == MAP)
		return extract_map( head, stream );

	return false;
}

int argo(json* dst, FILE* stream) {
	if ( !parse( dst, stream )) return -1;
	return 1;
}
