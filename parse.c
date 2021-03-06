#include "9cc.h"

// If a next token is expected symbol, update token to next one.
// Otherwise report an error.
void expect(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "Not '%s'", op);
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

// If a next token is expected symbol, update token to next one and return true.
// Otherwise return false.
bool consume(char *op) {
  if (token->kind != TK_RESERVED && token->kind != TK_RETURN ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

Token *consume_ident() {
  Token *tok = NULL;
  if (token->kind == TK_IDENT) {
    tok = token;
    token = token->next;
  }
  return tok;
}

// Search local variable by it's name.
// In the case that the local variable doesn't exist before, return NULL.
LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) 
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) 
      return var;
  return NULL;
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

  Token *tok = consume_ident();
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    
    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals->offset + 8;
      node->offset = lvar->offset;
      locals = lvar;
    }

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

Node *assign() {
  Node *node = equality();
  if(consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

Node *expr() {
  return assign();
}

Node *stmt() {
  Node *node;

  if (consume("return")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  } else {
    node = expr();
  }

  if (!consume(";"))
    error_at(token->str, "Not \';\' token.");

  return node;
}

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}
