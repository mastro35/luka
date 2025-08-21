// SPDX-License-Identifier: GPL-2.0
/* luka.c
 *
 * A simple RPN calculator for your terminal
 * made with love in Italy.
 *
 * Copyright 2025 Davide Mastromatteo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#define APP_VERSION_MAJOR 0
#define APP_VERSION_MINOR 4
#define APP_VERSION_PATCH 1 
#define COPYRIGHT "2025 Davide Mastromatteo"

// Helper macros to stringify values
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define APP_VERSION STR(APP_VERSION_MAJOR) "." STR(APP_VERSION_MINOR) "." STR(APP_VERSION_PATCH)

// Configurable elements

// Stack
#define INITIAL_STACK_LENGTH 10
#define INCREMENT_STACK_STEP 10
#define MAX_STACK_LENGTH 99
#define MAX_VIEWABLE_STACK 16

// History
#define INITIAL_HISTORY_LENGTH 10
#define INCREMENT_HISTORY_STEP 10
#define HISTORY_MAX_VIEWABLE_ELEMENTS 17

// Memory
#define INITIAL_MEMORIES_LENGTH 10
#define MAX_MEMORIES_LENGTH 99
#define INCREMENT_MEMORIES_STEP 5
#define MEMORY_MAX_VIEWABLE_ELEMENTS 17
#define MAX_MEMORY_NAME_LENGTH 10

// Modes
#define INITIAL_MODE 'r'
#define INITIAL_NUMERIC_FORMAT 's'
#define INITIAL_HISTORY_MODE 'l'

//UI 
#define PROMPT_POSITION 24
#define ERROR_POSITION 23
#define MAX_INPUT_BUFFER 100

// Standard includes needed by the program
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <getopt.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

// Definition of the global variables
char mode = INITIAL_MODE;
char numeric_format = INITIAL_NUMERIC_FORMAT;
char history_mode = INITIAL_HISTORY_MODE;

// Variables used for memories
double *values = NULL; 
char **memories = NULL; 
int n_memories = 0;
int current_memories_length = INITIAL_MEMORIES_LENGTH;

// Variables used for history
char **operation_log = NULL;
int n_operation_log = 0;
int current_history_length = INITIAL_HISTORY_LENGTH;

// Variables used for stack
double *stack = NULL;
int sp = 0;
int current_stack_length = INITIAL_STACK_LENGTH;

// UI
int history_view_offset = 0;
int memory_view_offset = 0;
char error_buffer[70];

// Local includes
#include "luka_stack.c"
#include "luka_functions.c"
#include "luka_ui.c"

// Function Pointers
operation_0o get_operation_0o(char *operation); 
operation_1o get_operation_1o(char *operation); 
operation_0o_with_parameter get_operation_0o_with_parameter(char *operation);
operation_1o get_trigonometric_operation_1o(char *operation); 
operation_2o get_operation_2o(char *operation); 

/* ************
   MAIN PROGRAM
   ************ */

/* Check the input inserted by the user in memory */
int check_input_if_numeric(char* input, double* value) {
  char* endptr;
  *value = strtod(input, &endptr);
  return endptr[0] == '\0';
}

/* Set Mode:
   d = DEG mode
   r = RAD mode */
void set_mode(char input_mode) {
  if (input_mode == 'r' || input_mode == 'd') mode = input_mode;
}

/* Set Numeric Format:
   f = Fixed Decimal format
   s = Scientific format */
void set_numeric_format(char input_format) {
  if (input_format == 's' || input_format == 'f') numeric_format = input_format;
}

/* Set the RAD mode */
void set_rad_mode(void) {
  set_mode('r');
}

/* Set the DEG mode */
void set_deg_mode(void) {
  set_mode('d');
}

/* Set the scientific numeric format */
void set_sci_numeric_format(void) {
  set_numeric_format('s');
}

/* Set the fixed numeric format */
void set_fix_numeric_format(void) {
  set_numeric_format('f');
}

/* Set the right panel to display the 
   operation history or the memories 
   depending on the current configuration */
void set_history_mode(char input_mode) {
  if (input_mode == 'l' || input_mode == 'm') history_mode = input_mode;
}

/* Set the operation history mode for the right panel */
void set_log_history_mode(void) {
  set_history_mode('l');
}

/* Set the memory mode for the right panel */
void set_memory_history_mode(void) {
  set_history_mode('m');
}

/* Compute the command received */
int compute(char* input) {
  double value = 0;
  operation_2o operation_2o = NULL;
  operation_1o operation_1o = NULL;
  operation_0o_with_parameter operation_0o_with_parameter = NULL;
  operation_0o operation_0o = NULL;

  char parameter[100] = "";
  char command[100] = "";

  char* token = strtok(input, " ");
  int i = 0;
  while (token != NULL) {
    if (i == 0) strcpy(command, token); else strcpy(parameter, token);    
    token = strtok(NULL, " ");
    i++;
  }

  /* If the input is numeric just push it to the stack
     and return */
  if (check_input_if_numeric(command, &value)) {
    push(value);
    return 0;
  }

  // Try to see if the command is a two operand operation
  if ((operation_2o = get_operation_2o(command))) {
    compute_operation_2o(operation_2o, command);
  }

  // Try to see if the command is a single operand operation
  if ((operation_1o = get_operation_1o(command))) {
    compute_operation_1o(operation_1o, command);
  }

  // Try to see if the command is a single operand trigonometric operation
  if ((operation_1o = get_trigonometric_operation_1o(command))) {
    compute_trigonometric_operation_1o(operation_1o, command);
  }

  // Try to see if the command is a no operand operation with parameter
  if ((operation_0o_with_parameter = get_operation_0o_with_parameter(command))) {
    compute_operation_0o_with_parameter(operation_0o_with_parameter, parameter);
  }

  // Try to see if the command is a no operand operation
  if ((operation_0o = get_operation_0o(command))) {
    compute_operation_0o(operation_0o);
  }
    
return 0;
}

/* Enable terminal raw mode to get arrows from keyboard 
   needed by the power_fgets */
void enable_raw_mode(struct termios *old_termios) {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, old_termios);
    new_termios = *old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

/* Disable terminal raw mode 
   needed by the power_fgets */
void disable_raw_mode(struct termios *old_termios) {
    tcsetattr(STDIN_FILENO, TCSANOW, old_termios);
}

/* Get an input from the keyboard 
   taking care of escape characters */
void power_fgets(char *buffer, int max_len) {
    struct termios old_termios;
    enable_raw_mode(&old_termios);

    int i = 0;
    while (i < max_len - 1) {
        char c = getchar();

        if (c == 27) { // if an escape char has been pressed...
          char seq1 = getchar();
          if (seq1 == '[') {
            char seq2 = getchar();
            switch (seq2) {
            case 'A': strcpy(buffer, "arrow_up\0"); break; 
            case 'B': strcpy(buffer, "arrow_down\0"); break;
            case 'C': strcpy(buffer, "arrow_right\0"); break;
            case 'D': strcpy(buffer, "arrow_left\0"); break;
            default: break;
            }
          }            
          break;
        } else {
          if (c == '\n') { 
            putchar('\n');
            break;
          }
          
          if (c == 127 || c == 8) { // this is the backspace
            if (i > 0) {
              i--;
              printf("\b \b");
              buffer[i] = '\0';
            }
            continue;
          } 
          
          buffer[i++] = c;
          putchar(c);
        }
        
        buffer[i] = '\0';
    }
    
    disable_raw_mode(&old_termios);
}

/* Get the user input */
void get_input(char* input) {
    locate(1,PROMPT_POSITION);
    printf("─────────\n");
    printf("‣ ");
    power_fgets(input, MAX_INPUT_BUFFER - 1);

    // to lower case
    while (*input) {
        *input = tolower((unsigned char)*input);
        input++;
    }

    input[strcspn(input, "\n")] = '\0';
}

/* Process the command line input */ 
void handle_command_line_parameters(int argc, char* argv[]) {
  int opt = 0;
  int option_index = 0;

  static struct option long_options[] = {
    {"deg", no_argument, 0, 'd'},
    {"rad", no_argument, 0, 'r'},
    {"sci", no_argument, 0, 's'},
    {"fix", no_argument, 0, 'f'},
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'V'},
    {0, 0, 0, 0}
  };

  while ((opt = getopt_long(argc, argv, "drsfV", long_options, &option_index))!=-1) {
    switch(opt) {
      case 'd': set_mode('d'); break;
      case 'r': set_mode('r'); break;
      case 's': set_numeric_format('s'); break;
      case 'f': set_numeric_format('f'); break;
      case 'h': show_command_line_help(); exit(0);
      case 'V': show_version(); exit(0);
      case '?': exit(1);
    }
  }
}

/* Get the no operands operation corresponding
   to the command received as input */
operation_0o get_operation_0o(char* operation) {

  if ((strcmp(operation, "exit") == 0) ||
      (strcmp(operation, "quit") == 0) ||
      (strcmp(operation, "q") == 0)) {
    return exit_program;
  }

  if ((strcmp(operation, "credits") == 0) || 
      (strcmp(operation, "?") == 0)) {
    return show_credits;
  }

  if (strcmp(operation, "pi") == 0) {
    return push_pi;
  }

  if ((strcmp(operation, "random") == 0) || 
    (strcmp(operation, "rnd") == 0)) {
    return push_random;
  }

  if (strcmp(operation, "e") == 0) {
    return push_e;
  }

  if (strcmp(operation, "rad") == 0) {
    return set_rad_mode;
  }

  if (strcmp(operation, "deg") == 0) {
    return set_deg_mode;
  }

  if (strcmp(operation, "fix") == 0) {
    return set_fix_numeric_format;
  }

  if (strcmp(operation, "sci") == 0) {
    return set_sci_numeric_format;
  }
  
  if (strcmp(operation, "license") == 0) {
      return show_license_message;
  }

  if ((strcmp(operation, "help") == 0) ||
      (strcmp(operation, "h") == 0)) {
    return show_help;
  }

  if ((strcmp(operation, "clear") == 0) ||
     (strcmp(operation, "c") == 0)) {
    return clear;
  }

  if ((strcmp(operation, "drop") == 0) ||
     (strcmp(operation, "d") == 0)) {
    return drop;
  }

  if ((strcmp(operation, "swap") == 0) ||
     (strcmp(operation, "s") == 0)) {
    return swap;
  }

  if (strcmp(operation, "history") == 0) {
    return set_log_history_mode;
  }

  if (strcmp(operation, "memory") == 0) {
    return set_memory_history_mode;
  }


  if ((strcmp(operation, "roll") == 0) ||
      (strcmp(operation, "rroll") == 0) || 
     (strcmp(operation, "arrow_right") == 0)) {
    return rroll;
  }

  if ((strcmp(operation, "unroll") == 0) ||
      (strcmp(operation, "lroll") == 0) || 
     (strcmp(operation, "arrow_left") == 0)) {
    return lroll;
  }

  if (strcmp(operation, "arrow_up") == 0) {
    if (history_mode == 'l') {
      history_view_offset = history_view_offset + 1;
    }

    if (history_mode == 'm') {
      memory_view_offset = memory_view_offset + 1;
    }
    return view_status;
  }

  if (strcmp(operation, "arrow_down") == 0) {
    if (history_mode == 'l') {
      history_view_offset = history_view_offset - 1;
      if (history_view_offset < 0) history_view_offset = 0;
    }

    if (history_mode == 'm') {
      memory_view_offset = memory_view_offset - 1;
      if (memory_view_offset < 0) memory_view_offset = 0;
    }
    return view_status;
  }

  return NULL;
}

/* Get the no operands operation with parameter corresponding
   to the command received as input */
operation_0o_with_parameter get_operation_0o_with_parameter(char *operation) {
  if (strcmp(operation, "store") == 0) {
    return store;}

  if (strcmp(operation, "load") == 0) {
    return load;}

  if (strcmp(operation, "del") == 0) {
    return del;}

  return NULL;
}

/* Get the single operand operation corresponding
   to the command received as input */
operation_1o get_operation_1o(char *operation) {
  if (strcmp(operation, "!") == 0) {
    return factorial;}

  if (strcmp(operation, "sqrt") == 0) {
    return sqrt;}

  if (strcmp(operation, "log10") == 0) {
    return log10;}

  if ((strcmp(operation, "log") == 0) || 
      (strcmp(operation, "ln") == 0)) {
    return log;}

  if ((strcmp(operation, "reciprocal") == 0) ||
      (strcmp(operation, "\\") == 0) ||
      (strcmp(operation, "rec") == 0)) {
    return reciprocal;}

  return NULL;
}

/* Get the single operand operation corresponding
   to the command received as input */
operation_1o get_trigonometric_operation_1o(char *operation) {
  if (strcmp(operation, "sin") == 0) {
    return sin;}

  if (strcmp(operation, "cos") == 0) {
    return cos;}

  if (strcmp(operation, "tan") == 0) {
    return tan;}

  if (strcmp(operation, "asin") == 0) {
    return asin;}

  if (strcmp(operation, "acos") == 0) {
    return acos;}

  if (strcmp(operation, "atan") == 0) {
    return atan;}

  return NULL;
}

/* Get the two-operands operation corresponding
   to the command received as input */
operation_2o get_operation_2o(char *operation) {
  if (strcmp(operation, "+") == 0) {
    return sum;}

  if (strcmp(operation, "-") == 0) {
    return subtraction;}

  if (strcmp(operation, "*") == 0) {
    return multiplication;}

  if (strcmp(operation, "/") == 0) {
    return division;}

  if ((strcmp(operation, "power") == 0) ||
      (strcmp(operation, "pow") == 0) ||
      (strcmp(operation, "^") == 0)) {
    return to_power;}

  return NULL;
}

/* Free all the pointers pointing the heap */ 
void free_pointers(void) {
  free(stack);
  free(operation_log);
  free(memories);
  free(values);
}

/* Entry point */
int main(int argc, char* argv[]) {
  char input[(MAX_INPUT_BUFFER-1)] = "";

  // Call the free_pointers function when exiting
  atexit(free_pointers);

  /* Randomize the seed 
     of the random number generator*/
  srand48(time(NULL));

  /* Allocate memory */
  operation_log = malloc(INITIAL_HISTORY_LENGTH * sizeof(char*));
  if (operation_log == NULL) {
    fprintf(stderr, "Failed to allocate operation log\n");
    exit(EXIT_FAILURE);
  }

  memories = malloc(INITIAL_MEMORIES_LENGTH * sizeof(char*));
  if (memories == NULL) {
    fprintf(stderr, "Failed to allocate memory names\n");
    exit(EXIT_FAILURE);
  }

  values = malloc(INITIAL_MEMORIES_LENGTH * sizeof(double));
  if (values == NULL) {
    fprintf(stderr, "Failed to allocate memory values\n");
    exit(EXIT_FAILURE);
  }

  stack = malloc(INITIAL_STACK_LENGTH * sizeof(double));
  if (stack == NULL) {
    fprintf(stderr, "Failed to allocate stack\n");
    exit(EXIT_FAILURE);
  }

  handle_command_line_parameters(argc, argv);

  // this is the REPL
  while (1) {                       // L
    view_status();                  // P
    get_input(input);               // R
    if (compute(input)) break;      // E
  }

  return 0;
}
