#include "9cc.h"

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

    if (strncmp(p, "<=", 2) == 0 || strncmp(p, ">=", 2) == 0 ||
        strncmp(p, "!=", 2) == 0 || strncmp(p, "==", 2) == 0) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }

    if(strncmp(p, "<", 1) == 0 || strncmp(p, ">", 1) == 0 ||
       strncmp(p, "+", 1) == 0 || strncmp(p, "-", 1) == 0 ||
       strncmp(p, "*", 1) == 0 || strncmp(p, "/", 1) == 0 ||
       strncmp(p, "(", 1) == 0 || strncmp(p, ")", 1) == 0 ||
       strncmp(p, "=", 1) == 0 || strncmp(p, ";", 1) == 0) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 1;
      p += 1;
      continue;
    }

    if ('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z') {
      char *lvalstart = p;
      int lvallen = 0;
      while (isalnum(*p) || *p == '_') {
        p++;
        lvallen++;
      }
      cur = new_token(TK_IDENT, cur, lvalstart);
      cur->len = lvallen;
      continue;
    }
    
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->len = 1;
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(cur->str, "Unable to tokenize");
  }

  new_token(TK_EOF, cur, p);

  token = head.next;
  return head.next;
}

