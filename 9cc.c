#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Kind of Tokens
typedef enum{
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
};

// The Token currently focused on
Token *token;

// The function to report an error
// Same arguments as one of printf
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// If a next token is expected symbol, update token to next one and return true.
// Otherwise return false.
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

// If a next token is expected symbol, update token to next one.
// Otherwise report an error.
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error("Not '%c'", op);
  token = token->next;
}

// If a next token is a number, update token to next one and return that number.
// Otherwise report an error.
int expect_number() {
  if (token->kind != TK_NUM) 
    error("Not number");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// Create new token and connect cur
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// Tokenize input string p and return them.
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // Skip spaces
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error("Unable to tokenize");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Number of arguments is invalid.\n");
    return 1;
  }
  
  // Tokenize
  token = tokenize(argv[1]);

  // Output first part of asembly
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // Check whether first of formula is number or not,
  // and print first mov instruction
  printf("  mov rax, %ld\n", expect_number());

  // Comsume order of tokens that '+ <num> or - <num>'
  // and print asembly
  while (!at_eof()) {
    if (consume('+')) {
      printf("  add rax, %ld\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %ld\n", expect_number());

  }

  printf("  ret\n");
  return 0;
}
