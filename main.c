#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Number of arguments is invalid.\n");
    return 1;
  }

  // Tokenize and parse
  // The results of tokenize are stored in code array
  user_input = argv[1];
  token = tokenize(user_input);
  program();
  
  
  // Output first part of asembly
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // Prologue
  // Allocation of local variables(a-z)
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  // Generate asembly code going down abstract syntax tree
  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // Since one value should remain on the top of stack as a result of syntax evaluation,
    // pop up the stack so that it doesn't overflow
    printf("  pop rax\n");
  }

  // Epilogue
  // Since the result value of last syntax evaluation should remain on the top of stack,
  // load the result value into rax and return it.
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
