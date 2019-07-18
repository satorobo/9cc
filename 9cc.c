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
  int len;        // Length of Token
};

// The Token currently focused on
Token *token;

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
bool consume(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// If a next token is expected symbol, update token to next one.
// Otherwise report an error.
void expect(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
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

    if (strncmp(p, "<=", 2) == 0 || strncmp(p, ">=", 2) == 0 ||
        strncmp(p, "!=", 2) == 0 || strncmp(p, "==", 2) == 0) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if(strncmp(p, "<", 1) == 0 || strncmp(p, ">", 1) == 0 ||
       strncmp(p, "+", 1) == 0 || strncmp(p, "-", 1) == 0 ||
       strncmp(p, "*", 1) == 0 || strncmp(p, "/", 1) == 0 ||
       strncmp(p, "(", 1) == 0 || strncmp(p, ")", 1) == 0 ) {
      cur = new_token(TK_RESERVED, cur, p, 1);
      p += 1;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 1);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(cur->str, "Unable to tokenize");
  }

  new_token(TK_EOF, cur, p, 1);
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

Node *term(){
  // If next token is "(", it must be "(" expr ")"
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  // Otherwise it must be number
  return new_node_num(expect_number());
}

Node *unary() {
  if (consume("+"))
    return term();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), term());
  return term();
}

Node *mul() {
  Node *node = unary();
  
  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }

}

Node *add() {
  Node *node = mul();
  
  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else
      return node;
  }
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else
      return node;
  }

}

Node *expr() {
  Node *node = equality();

  return node;
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
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
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
