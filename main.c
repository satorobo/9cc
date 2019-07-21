#include "9cc.h"

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
