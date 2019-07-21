#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Kind of Tokens
typedef enum {
  TK_RESERVED, // Token of Symbol
  TK_NUM,      // Token of Integer
  TK_EOF,      // Token of End Of File
} TokenKind;

typedef struct Token Token;
// Token struction
struct Token {
  TokenKind kind; // Type of Token 
  Token *next;    // Pointer to a next Token
  int val;        // If kind is a number(TK_NUM), that number
  char *str;      // Token string
  int len;        // Length of Token
};

// Kind of abstract syntax tree(AST)
typedef enum {
   ND_ADD, // +
   ND_SUB, // -
   ND_MUL, // *
   ND_DIV, // /
   ND_LT,  // <
   ND_LE,  // <=
   ND_EQ,  // ==
   ND_NE,  // !=
   ND_NUM, // Integer
} NodeKind;

typedef struct Node Node;
// Type of nodes of abstract syntax tree
struct Node {
  NodeKind kind; // Type of nodes
  Node *lhs;     // Left-hand side childe node
  Node *rhs;     // Right-hand side childe node
  int val;       // It is only used when kind is ND_NUM
};

Node *term();
Node *unary();
Node *mul();
Node *add();
Node *relational();
Node *equality();
Node *expr();

// Inputed program string
char *user_input;

// The Token currently focused on
Token *token;
Token *tokenize(char *p);

void gen(Node *node);

void error_at(char *loc, char *fmt, ...);
bool at_eof();
