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

// Kind of abstract syntax tree(AST)
typedef enum {
   ND_ADD, // +
   ND_SUB, // -
   ND_MUL, // *
   ND_DIV, // /
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

// Inputed program string
char *user_input;

// The function to report position of an error
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); 
  fprintf(stderr, "^ ");
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
    error_at(token->str, "Not '%c'", op);
  token = token->next;
}

// If a next token is a number, update token to next one and return that number.
// Otherwise report an error.
int expect_number() {
  if (token->kind != TK_NUM) 
    error_at(token->str, "Not number");
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

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(cur->str, "Unable to tokenize");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *mul(){
  Node *node = new_node_num(expect_number());
  
  for (;;) {
    if (consume('*'))
      node = new_node(ND_MUL, node, mul());
    else if (consume('/'))
      node = new_node(ND_DIV, node, mul());
    else
      return node;
  }

}

Node *expr() {
  Node *node = mul();
  
  for (;;) {
    if (consume('+'))
      node = new_node(ND_ADD, node, mul());
    else if (consume('-'))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:    
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rax, rdi\n");
    break;
  }

  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Number of arguments is invalid.\n");
    return 1;
  }

  // Tokenize and parse
  user_input = argv[1];
  token = tokenize(user_input);
  Node *node = expr();
  

  // Output first part of asembly
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // Generate asembly code going down abstract syntax tree
  gen(node);
  
  // Since the result value of whole formula should be at the top of the stack,
  // load the result value into rax and return it.
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
