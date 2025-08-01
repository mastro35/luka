// SPDX-License-Identifier: GPL-2.0
/* luka_functions.c
 *
 * A simple RPN calculator for terminal
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

void log_operation_2o(double, double, char*, double);
void log_operation_1o(double, char*, double);

/* *****************
   GENERIC FUNCTIONS
   ***************** */

/* Generic function pointers for the single operand operations,
   the two-operands operations and the no operand operations */
typedef void (*operation_0o)(void);
typedef void (*operation_0o_with_parameter)(char*);
typedef double (*operation_1o)(double);
typedef double (*operation_2o)(double, double);

/* Compute an operation that doesn't take any operands */
void compute_operation_0o(operation_0o f) {
  f();
}

/* Compute a single operand operation */
void compute_operation_1o(operation_1o f, char *name) {
  if (sp < 1) return;
  double x = pop();
  double r = f(x);
  push(r);
  log_operation_1o(x, name, r);
}

/* Compute a single trigonometric operation 
   It differs from the standard single operand operation
   because it may need to convert radians to dregrees */
void compute_trigonometric_operation_1o(operation_1o f, char *name) {
  if (sp < 1) return;
  double x = pop();
  if (mode == 'd') x = x * M_PI / 180;
  double r = f(x);
  push(r);
  log_operation_1o(x, name, r);
}

/* Compute a special operation that doesn't take any operand
   but accept a parameter (like the store or load operations) */
void compute_operation_0o_with_parameter(operation_0o_with_parameter f, char *parameter) { 
  f(parameter);
}

/* Compute a two-operands operation */
void compute_operation_2o(operation_2o f, char *name) {
  if (sp < 2) return;
  double y = pop();
  double x = pop();
  double r = f(y, x);
  push(r);
  log_operation_2o(y, x, name, r);
}

/* Generic function to log an entry passed to the function */
void log_operation(char* entry) {
  if (n_operation_log >= current_history_length) {
    
      // The shistory array need to be resized
      int new_history_length = current_history_length + INCREMENT_HISTORY_STEP;

      operation_log = realloc(operation_log, (new_history_length) * sizeof(char*));
      if (operation_log == NULL) {
        printf("ERROR: You run out of memory. Exiting.");
        exit(1);
      }
//      sprintf(error_buffer, "Your history log has been resized to %d", new_history_length);
      current_history_length = new_history_length;
  }
  operation_log[n_operation_log] = strdup(entry);
}

/* Log operations involving two operands*/
void log_operation_2o(double y, double x, char *name, double r) {
  char entry[50] = "";
  sprintf(entry, "%lg %s %lg = %lg", y, name, x, r);
  log_operation(entry);
  n_operation_log ++;
}

/* Log operations involving just a single operand*/
void log_operation_1o(double x, char *name, double r) {
  char entry[50] = "";
  sprintf(entry, "%lg %s = %lg", x, name, r);
  log_operation(entry);
  n_operation_log ++;
}

/* *****************
   Math Functions
   ***************** */

/* Compute the power x of y */
double to_power(double x, double y) {
  return pow(y, x);
}

/* Compute a sum between two numbers */
double sum(double x, double y) {
  return y + x;
}

/* Compute a subtraction between two numbers*/
double subtraction(double x, double y) {
  return y - x;
}

/* Compute a multiplication between two numbers */
double multiplication(double x, double y) {
  return x * y;
}

/* Compute a division between two numbers */
double division(double x, double y) {
  return y / x;
}

/* Compute the factorial of a number*/
double factorial(double x) {
  return tgamma(x+1);
}

/* Compute the reciprocal of a number*/
double reciprocal(double x) {
  return (1/x);
}

/* *********************
   Convenience Functions
   ********************* */

/* Push the PI value to the stack */
void push_pi(void) {
  push(M_PI);
}

/* Push the eulero's number to the stack */
void push_e(void) {
  push(M_E);
}

/* Push a random value between 0 and 1 to the stack */
void push_random(void) {
  push(drand48());
}

/* *****************************
   Standard Calculator Functions
   ***************************** */

/* Exit the program */
void exit_program(void) {
  exit(0);
}

/* Search the calculator memory to find a defined variable */
int search_memory(char *parameter) {
  for (int i=0; i < n_memories; i++) {
    if (strcmp(memories[i], parameter) == 0) {
      return i;
    }
  }
  return -1;
}

/* Store a value in the calculator memory */
void store(char *parameter) {
  int i = -1;
  char buffer[10];

  if (strlen(parameter) > MAX_MEMORY_NAME_LENGTH) {
    sprintf(error_buffer, "ERROR: Memory names can be at maximum %d bytes length", MAX_MEMORY_NAME_LENGTH);
    return;
  }

  if ((i = search_memory(parameter)) == -1) {

    if (n_memories >= current_memories_length) {

      // We have too elements in the stack and we can't grow more
      if (current_memories_length == MAX_MEMORIES_LENGTH) {
        sprintf(error_buffer, "ERROR: You can't memorize more than %d entries", current_memories_length);
        return;
      }

      // Memories need to be resized
      int new_memories_length = current_memories_length + INCREMENT_MEMORIES_STEP;
      if (new_memories_length > MAX_MEMORIES_LENGTH) new_memories_length = MAX_MEMORIES_LENGTH;

      memories = (char**)realloc(memories, (new_memories_length * sizeof(char*)));
      values = (double*)realloc(values, (new_memories_length * sizeof(double)));

      if ((memories == NULL) || (values == NULL)) {
        printf("ERROR: You run out of memory. Exiting.");
        exit(1);
      }

      current_memories_length = new_memories_length;
    }

    i = n_memories;
    n_memories++;
  }

  ((double*)values)[i] = pick(sp);
  strcpy(buffer, parameter);
  memories[i] = strdup(buffer);
}

/* Load a value from the calculator memory and push it into the stack */
void load(char *parameter) {
  for (int i=0; i < n_memories; i++) {
    if (strcmp(memories[i], parameter) == 0) {
      push(((double*)values)[i]);
      return;
    }
  }
}

/* Remove a value from the calculator memory */
void del(char *parameter) {
  int k=0;
  for (int i=0; i < n_memories; i++) {
    if (k == 1) {
      memories[i - 1] = memories[i];
      ((double*)values)[i - 1] = ((double*)values)[i];
    }

    if (strcmp(memories[i], parameter) == 0) k = 1;
  }  
  n_memories -= k;
}
