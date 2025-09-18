#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct node {
    enum {
        ADD,
        MULTI,
        VAL
    }   type;
    int val;
    struct node *l;
    struct node *r;
}   node;

void    destroy_tree(node *n);

node *make_node(int type, int val, node *l, node *r) {
    node *n = malloc(sizeof(*n));
    if (!n) {
		destroy_tree(l);
		destroy_tree(r);
		return NULL;
	}
    n->type = type; n->val = val;
    n->l = l; n->r = r;
    return n;
}

void    destroy_tree(node *n)
{
    if (!n)
        return ;
    if (n->type != VAL)
    {
        destroy_tree(n->l);
        destroy_tree(n->r);
    }
    free(n);
}

int eval_tree(node *tree)
{
    switch (tree->type)
    {
        case ADD:
            return (eval_tree(tree->l) + eval_tree(tree->r));
        case MULTI:
            return (eval_tree(tree->l) * eval_tree(tree->r));
        case VAL:
            return (tree->val);
    }
}

void    unexpected(char c)
{
    if (c)
        printf("Unexpected token '%c'\n", c);
    else
        printf("Unexpected end of file\n");
}

node *parse ( char* exp );

int main( int ac, char** av ) {
	if (ac != 2) return 1;
	
	char* exp = av[1];
	node* tree = parse ( exp );
	if (!tree) return 1;
	printf("%d\n", eval_tree(tree));
	destroy_tree(tree);
	return 0;
}

typedef struct
	{ char* exp; int len; int pos; char current; }
lexer;

void advance ( lexer* lex ) {
	if (lex->pos < lex->len) {	
		lex->pos++;
		lex->current = lex->exp[lex->pos];
	}
}

char peek ( lexer* lex ) {
	return lex->current;
}

node* __exp ( lexer* lex );

node* __term( lexer* lex ) {
	if (isdigit(peek( lex ))) {
		node* val = make_node(VAL, peek( lex ) - '0', NULL, NULL);
		advance( lex );
		return val;
	}
	if ( peek( lex ) == '(' ) {
		advance( lex );
		node* sub = __exp( lex );
		if ( !sub || peek( lex ) != ')' ) return NULL;
		advance( lex );
		return sub;
	}
	return NULL;
}

node* __factor ( lexer* lex ) {
	node* left = __term(lex);
	if (!left) return NULL;
	while ( peek(lex) == '*' ) {
		advance( lex );
		node* right = __term( lex );
		if (!right) { destroy_tree(left); return NULL; }
		left = make_node(MULTI, 0, left, right);
		if (!left) return NULL;
	}	
	return left;
}

node* __exp ( lexer* lex ) {
	node *left = __factor(lex);
	if (!left) return NULL;
	while ( peek(lex) == '+' ) {
		advance(lex);
		node* right = __factor(lex);
		if (!right) { destroy_tree(left); return NULL; }
		left = make_node(ADD, 0, left, right);
		if (!left) return NULL;
	}
	return left;
}

node *parse ( char* exp ) {

	lexer lex = { exp, strlen(exp), 0, exp[0] };

	node *tree = __exp( &lex );
	if ( !tree || peek( &lex ) != 0 ) {
		destroy_tree(tree);
		unexpected(peek( &lex ));
		return NULL;
	}
	return tree;
}
