#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


// learn parsing on a real project some prefix tree, a lang lexer, some scripting syntax parser not a bullshit 1+1 use a fucking calculator

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

int parse ( char* exp );

int main( int ac, char** av ) {
	if (ac != 2) return 1;
	
	char* exp = av[1];
	int lol = parse (exp);
	if (lol != -1)
		printf("%d\n", lol);
	// node* tree = parse ( exp );
	// if (!tree) return 1;
	// printf("%d\n", eval_tree(tree));
	// destroy_tree(tree);
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

void back ( lexer* lex ) {
	if (lex->pos > 0) {	
		lex->pos--;
		lex->current = lex->exp[lex->pos];
	}
}

char peek ( lexer* lex ) {
	return lex->current;
}

int __exp ( lexer* lex );

int __term( lexer* lex ) {
	if (isdigit(peek(lex))) {
		int tmp = peek(lex) - '0';
		advance(lex);
		return tmp;
	}
	else if (peek(lex) == '(') {
		advance(lex);
		int sub_lol = __exp(lex);
		if (sub_lol == -1) return -1;
		if (peek(lex) != ')') return -1;
		advance(lex);
		return sub_lol;
	}
	return -1; // think about errors later
}

int __factor ( lexer* lex ) {
	int left = __term(lex);
	if (left == -1) return -1;
	// multiplication will go here soon
	while (peek(lex) == '*') {
		advance(lex);
		int right = __term(lex);
		if (right == -1) return -1;
		left = left * right;
	}
	return left;
}

int __exp ( lexer* lex ) {
	int left = __factor(lex);
	if (left == -1) return -1;
	while (peek(lex) == '+') {
		advance(lex);
		int right = __factor(lex);
		if (right == -1) return -1;
		left = left + right;
	}
	return left;
}

int parse ( char* exp ) {

	lexer lex = { exp, strlen(exp), 0, exp[0] };

	int lol = 0;
	lol = __exp(&lex);
	printf("last char pointer at %c\n", lex.current);
	if (peek(&lex) != 0 || lol == -1) {
		unexpected(peek(&lex));
		return -1;
	}
	// node *tree = __exp( &lex );
	// if ( !tree || peek( &lex ) != 0 ) {
	// 	destroy_tree(tree);
	// 	unexpected(peek( &lex ));
	// 	return NULL;
	// }
	return lol;
}
