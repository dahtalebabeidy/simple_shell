#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024
#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

extern char **environ;

char *custom_getline();
void display_prompt();
int parse_input(char *input, char *commands[]);
void print_environment();
int custom_setenv(const char *name, const char *value, int overwrite);
int custom_unsetenv(const char *name);
int custom_cd(const char *dir);
int execute_command(char *command);

#endif
