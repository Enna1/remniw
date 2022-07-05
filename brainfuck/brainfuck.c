/* borrowed from https://onestepcode.com/brainfuck-compiler-c/ */

#include <stdio.h>

#define DATASIZE 1001

void brainfuck(char *command_pointer, char *input) {
  int bracket_flag;
  char command;
  int command_pc = 0;
  int input_ptr = 0;
  char data[DATASIZE] = {0};
  int data_ptr =
      DATASIZE / 2; /* data pointer, Move dp to middle of the data array */

  while (1) {
    command = command_pointer[command_pc];
    command_pc++;
    if (!command)
      break;
    switch (command) {
    case '>': /* Move data pointer to next address */
      data_ptr++;
      break;
    case '<': /* Move data pointer to previous address */
      data_ptr--;
      break;
    case '+': /* Increase value at current data cell by one */
      data[data_ptr]++;
      break;
    case '-': /* Decrease value at current data cell by one */
      data[data_ptr]--;
      break;
    case '.': /* Output character at current data cell */
      printf("%c", data[data_ptr]);
      break;
    case ',': /* Accept one character from input pointer ip and
                 advance to next one */
      data[data_ptr] = input[input_ptr];
      input_ptr++;
      break;
    case '[': /* When the value at current data cell is 0,
                 advance to next matching ] */
      if (!data[data_ptr]) {
        for (bracket_flag = 1; bracket_flag; command_pc++)
          if (command_pointer[command_pc] == '[')
            bracket_flag++;
          else if (command_pointer[command_pc] == ']')
            bracket_flag--;
      }
      break;
    case ']': /* Moves the command pointer back to matching
                 opening [ if the value of current data cell is
                 non-zero */
      if (data[data_ptr]) {
        /* Move command pointer just before ] */
        command_pc -= 2;
        for (bracket_flag = 1; bracket_flag; command_pc--)
          if (command_pointer[command_pc] == ']')
            bracket_flag++;
          else if (command_pointer[command_pc] == '[')
            bracket_flag--;
        /* Advance pointer after loop to match with opening [ */
        command_pc++;
      }
      break;
    }
  }
  /* Adding new line after end of the program */
  printf("\n");
}

int main() {
  char *hello_world_code =
      "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>"
      ".<-.<.+++.------.--------.>>+.>++.";
  char *input = "";
  brainfuck(hello_world_code, input);
  return 0;
}