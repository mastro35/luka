// SPDX-License-Identifier: GPL-2.0
/* luka_ui.c
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

/* Locate the cursor in a specific position */
void locate(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

/* Returns the register name to be displayed for a stack position */
void get_register_name(int i, char* buffer) {
  sprintf(buffer, "%2d", i);

  if (i==1) strcpy(buffer," x");
  if (i==2) strcpy(buffer," y");
}

/* Display the command line help when called with the --help parameter */
void show_command_line_help(void) {
    printf("Usage: luka [OPTION]...\n");
    printf("luka - a simple terminal-based RPN calculator\n\n");

    printf("Options:\n");
    printf("  -d, --deg          Set angle mode to degrees\n");
    printf("  -r, --rad          Set angle mode to radians (default)\n");
    printf("  -s, --sci          Use scientific notation for numbers (default)\n");
    printf("  -f, --fix          Use fixed-point notation for numbers\n");
    printf("  -V, --version      Show version information and exit\n");
    printf("  -h, --help         Display this help message and exit\n\n");

    printf("Examples:\n");
    printf("  luka --deg --fix     Start in degrees mode with fixed-point display\n");
    printf("  luka -s              Start with scientific display mode\n\n");

    printf("This is free software released under the GNU GPL v2.\n");
    printf("Made with love in Italy by Davide Mastromatteo\n");
}

/* Display the version of the program */
void show_version(void) {
  printf("luka %s Copyright (C) %s\n", APP_VERSION, COPYRIGHT);
}

/* Display a message showing the license of the program */
void show_license_message(void) {
  printf("\x1B[1;1H\x1B[2J");
  show_version();
  printf("luka comes with ABSOLUTELY NO WARRANTY. \n");  
  printf("This is free software, and you are welcome to redistribute it\n");
  printf("under certain conditions.\n");
  printf("Check the license at https://www.gnu.org/licenses/old-licenses/gpl-2.0.html\n");
  printf("\n");
  printf("press ENTER to continue\n");
  getchar();
}

/* Show the memories panel */
void show_memories(void) {
  int k = 0;
  locate (40, 4);
  printf("──────MEMORY─────\n");

  int begin = (n_memories - MEMORY_MAX_VIEWABLE_ELEMENTS - memory_view_offset) > 0 ? n_memories - MEMORY_MAX_VIEWABLE_ELEMENTS - memory_view_offset: 0;
  int end = (begin + MEMORY_MAX_VIEWABLE_ELEMENTS);

  if ((end + memory_view_offset > n_memories) && (memory_view_offset > 0)) memory_view_offset--;
  if (end > n_memories) end = n_memories;

  if (begin > 0) {
    locate (41, 4);
    printf("⇡");    
  }

  for (int i = begin; i < end; i++) {
    if (strcmp(memories[i], "") == 0) continue;
    locate (40, (5 + (k++)));
    if (numeric_format == 's') printf("%s - %lg", memories[i], values[i]);    
    if (numeric_format == 'f') printf("%s - %lf", memories[i], values[i]);    
  }

  if (end < n_memories) {
    locate (41, (6 + k - 1));
    printf("⇣");    
  } 
}


/* Show the operations history panel */
void show_history(void) {
  int k = 0;
  locate (40, 4);
  printf("──────HISTORY─────\n");

  int begin = (n_operation_log - HISTORY_MAX_VIEWABLE_ELEMENTS - history_view_offset) > 0 ? n_operation_log - HISTORY_MAX_VIEWABLE_ELEMENTS - history_view_offset: 0;
  int end = (begin + HISTORY_MAX_VIEWABLE_ELEMENTS);

  if ((end + history_view_offset > n_operation_log) && (history_view_offset > 0)) history_view_offset--;
  if (end > n_operation_log) end = n_operation_log;

  if (begin > 0) {
    locate (41,4);
    printf("⇡");    
    fflush(stdout);
  }

  for (int i = begin; i < end; i++) {
    locate (40, (5 + (k++)));
    printf("%4d │ %s\n", (i + 1), operation_log[(i)]);
  }

  if (end < n_operation_log) {
    locate (41, (6 + k - 1));
    printf("⇣");    
    fflush(stdout);
  } 

}


/* Print a nicely formatted value of the stack
   depending on the numeric_format set */
void print_stack_value(char* buffer, double number) {
  double abs_number = number < 0 ? number * -1 : number; 
  if ((abs_number >= 1e10) || (abs_number > 0 && abs_number < 1e-6)) {
    printf("│ %s │ %25.15e│\n", buffer, number);
  } else {
    if (numeric_format == 'f') {
      printf("│ %s │ %25.6f│\n", buffer, number);
    }
    else {
      printf("│ %s │ %25.15g│\n", buffer, number);
    }
  }
}

/* Shows the calculator current set mode */
void show_rpn_modes(void) {
  char mode_string[] = "err";
  if (mode == 'd') strcpy(mode_string, "deg");
  if (mode == 'r') strcpy(mode_string, "rad");

  char numeric_format_string[] = "err";
  if (numeric_format == 'f') strcpy(numeric_format_string, "fix");
  if (numeric_format == 's') strcpy(numeric_format_string, "sci");

  printf("┌─────┬─────┐ \n");	
  printf("│ %s │ %s │ \n", mode_string, numeric_format_string);
  printf("└─────┴─────┘ \n");	
}

/* Shows the calculator Stack */
void show_stack(void) {
  printf("┌────┬──────────STACK───────────┐\n");

  char buffer[3];

  int start = 0;
  if (sp > MAX_VIEWABLE_STACK) {
    start = sp - (MAX_VIEWABLE_STACK - 1);
    get_register_name((sp) , buffer);
    print_stack_value(buffer, stack[sp - 1]);
    printf("│....│..........................│\n");
  }

  for (int i=start; i<sp; i++) {
    get_register_name((sp) - i, buffer);
    print_stack_value(buffer, stack[i]);
  } 
  printf("└────┴──────────────────────────┘\n");
}

/* Shows the lateral panel, depending on what the
   user decided to shows */
void show_lateral_panel(void) {
  switch (history_mode) {
    case 'l':show_history(); break;
    case 'm':show_memories(); break;
  }
}

void show_errors(void) {
  locate(1, ERROR_POSITION);
  printf("%s", error_buffer);
  error_buffer[strcspn(error_buffer, "\n")] = '\0';
  strcpy(error_buffer, "");
}

/* Clear the screen and 
   shows the status of the calculator */
void view_status(void) {
  printf("\x1B[1;1H\x1B[2J");

  show_rpn_modes();
  show_stack();
  show_lateral_panel();
  show_errors();
}

/* Shows the credits window */
void show_credits(void) {
  printf("\x1B[1;1H\x1B[2J");
  printf("\n");
  printf("luka\n");
  printf("ˆˆˆˆ\n");
  printf("made with love in Italy\n");
  printf("<mastro35@gmail.com> || https://hachyderm.io/@mastro35\n");
  printf("If you enjoy this program buy me a coffee at\n");
  printf("https://buymeacoffee.com/mastro35\n\n");
  printf("LICENSE INFORMATION\n");
  printf("ˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆ\n");
  show_version();
  printf("luka comes with ABSOLUTELY NO WARRANTY. \n");  
  printf("This is free software, and you are welcome to redistribute it\n");
  printf("under certain conditions.\n");
  printf("Check the license at https://www.gnu.org/licenses/old-licenses/gpl-2.0.html\n");
  printf("\n\n");
  printf("press ENTER to continue\n");
  getchar();
}

/* Shows the help screen */
void show_help(void) {
    printf("\x1B[1;1H\x1B[2J"); // Clear screen
    printf("luka - RPN Calculator v%s\n", APP_VERSION);
    printf("──────────────────────────────────────────────────────\n");

    printf(" Basic Ops:     +  -  *  /  ^\n");
    printf(" Stack Ops:     d(drop)   s(swap)   c(clear)\n");
    printf(" Rotate Stack:  roll      unroll    ← → (keys)\n");
    printf(" Stack View:    ↑ ↓ (view history/memory)\n\n");

    printf(" Functions:\n");
    printf("  sqrt  log  ln  log10  ! (factorial)  \\ (recip)\n");
    printf("  sin  cos  tan  asin  acos  atan\n\n");
    printf(" Modes: deg / rad       Format: fix / sci\n\n");

    printf(" Constants:     pi   e   rnd (random)\n");
    printf(" Memory:        store [name]   load [name]   del [name]\n\n");

    printf(" Commands:\n");
    printf("  ENTER      Repeat last input\n");
    printf("  h/help     Show this help screen\n");
    printf("  ?          Show credits and license\n");
    printf("  q/quit     Exit program\n");

    printf("──────────────────────────────────────────────────────\n");
    printf(" Press ENTER to return...");
    getchar();
}
