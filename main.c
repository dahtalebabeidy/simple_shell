#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024
#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

extern char **environ; // External variable containing the environment

char *custom_getline() {
    static char buffer[BUFFER_SIZE];
    static size_t buffer_index = 0;
    static ssize_t bytes_read = 0;

    // If the buffer is empty, read more data
    if (buffer_index >= bytes_read || bytes_read == 0) {
        bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));

        // If read returns 0, we have reached the end of the input
        if (bytes_read <= 0) {
            return NULL;
        }

        buffer_index = 0;
    }

    // Find the end of the line or end of the buffer
    size_t i = buffer_index;
    while (i < bytes_read && buffer[i] != '\n') {
        i++;
    }

    // Allocate memory for the line
    size_t line_length = i - buffer_index;
    char *line = (char*)malloc(line_length + 1);

    // Copy the line from the buffer to the allocated memory
    for (size_t j = 0; j < line_length; j++) {
        line[j] = buffer[buffer_index + j];
    }

    // Null-terminate the line
    line[line_length] = '\0';

    // Move the buffer_index to the next character after the newline
    buffer_index = i + 1;

    return line;
}

void display_prompt() {
    printf("simple_shell$ ");
    fflush(stdout);
}

int parse_input(char *input, char *args[]) {
    int i = 0;
    char *token = strtok(input, " ");

    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }

    args[i] = NULL; // Set the last element to NULL for execvp
    return i;
}

void print_environment() {
    char **env = environ;
    while (*env != NULL) {
        printf("%s\n", *env);
        env++;
    }
}

int custom_setenv(const char *name, const char *value, int overwrite) {
    if (name == NULL || value == NULL) {
        fprintf(stderr, "setenv: Missing argument\n");
        return -1;
    }

    char **env = environ;
    while (*env != NULL) {
        if (strncmp(*env, name, strlen(name)) == 0 && (*env)[strlen(name)] == '=') {
            if (!overwrite) {
                fprintf(stderr, "setenv: Variable '%s' already exists\n", name);
                return -1;
            } else {
                // Update the existing variable
                free(*env);
                *env = malloc(strlen(name) + strlen(value) + 2); // +1 for '=' and +1 for null terminator
                sprintf(*env, "%s=%s", name, value);
                return 0;
            }
        }
        env++;
    }

    // Variable doesn't exist, create a new one
    char **new_env = malloc((env - environ + 2) * sizeof(char *));
    memcpy(new_env, environ, (env - environ) * sizeof(char *));
    new_env[env - environ] = malloc(strlen(name) + strlen(value) + 2);
    sprintf(new_env[env - environ], "%s=%s", name, value);
    new_env[env - environ + 1] = NULL;

    free(environ);
    environ = new_env;

    return 0;
}

int custom_unsetenv(const char *name) {
    if (name == NULL) {
        fprintf(stderr, "unsetenv: Missing argument\n");
        return -1;
    }

   char **env = environ;
    char **new_env = environ;

    while (*env != NULL) {
        if (strncmp(*env, name, strlen(name)) == 0 && (*env)[strlen(name)] == '=') {
            // Skip the variable to remove it
            env++;
            continue;
        }

        *new_env = *env;
        new_env++;
        env++;
    }

    *new_env = NULL;

    return 0;
}

int custom_cd(const char *dir) {
    const char *new_dir; // Declare new_dir as const pointer

    if (dir == NULL || strcmp(dir, "") == 0) {
        // If no argument is given, change to the home directory
        new_dir = getenv("HOME");
    } else if (strcmp(dir, "-") == 0) {
        // If the argument is "-", change to the previous directory
        new_dir = getenv("OLDPWD");
        if (new_dir == NULL) {
            fprintf(stderr, "cd: OLDPWD not set\n");
            return -1;
        }
    } else {
        new_dir = dir;
    }

    // Get the current working directory for updating PWD and OLDPWD
    char current_dir[BUFFER_SIZE];
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("getcwd");
        return -1;
    }

    // Update OLDPWD
    if (setenv("OLDPWD", current_dir, 1) != 0) {
        perror("setenv");
        return -1;
    }

    // Change to the new directory
    if (chdir(new_dir) != 0) {
        perror("chdir");
        return -1;
    }

    // Update PWD
    if (setenv("PWD", new_dir, 1) != 0) {
        perror("setenv");
        return -1;
    }

    return 0;
}

int main() {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARGS];

    while (1) {
        // Display prompt and read user input
        display_prompt();
        if (fgets(input, sizeof(input), stdin) == NULL) {
            // Handle end of file (Ctrl+D)
            printf("\n");
            break;
        }

        // Remove the newline character at the end of input
        input[strcspn(input, "\n")] = '\0';

        // Check for the built-in commands
        if (strncmp(input, "exit", 4) == 0) {
            // Parse the exit command and extract the status code
            int status_code = 0;
            if (strlen(input) > 4) {
                status_code = atoi(input + 5);
            }

            printf("Exiting shell with status code: %d\n", status_code);
            exit(status_code);
        } else if (strcmp(input, "env") == 0) {
            print_environment();
            continue; // Skip the fork and wait steps for built-in commands
        } else if (strncmp(input, "setenv", 6) == 0) {
            // Parse the setenv command and call the custom_setenv function
            char *name = strtok(input + 6, " ");
            char *value = strtok(NULL, " ");
            if (name != NULL && value != NULL) {
                custom_setenv(name, value, 1);
            } else {
                fprintf(stderr, "setenv: Invalid syntax\n");
            }
            continue; // Skip the fork and wait steps for built-in commands
        } else if (strncmp(input, "unsetenv", 8) == 0) {
            // Parse the unsetenv command and call the custom_unsetenv function
            char *name = strtok(input + 8, " ");
            if (name != NULL) {
                custom_unsetenv(name);
            } else {
                fprintf(stderr, "unsetenv: Invalid syntax\n");
            }
            continue; // Skip the fork and wait steps for built-in commands
        } else if (strncmp(input, "cd", 2) == 0) {
            // Parse the cd command and call the custom_cd function
            char *dir = strtok(input + 2, " ");
            if (dir != NULL) {
                custom_cd(dir);
            } else {
                custom_cd(NULL);
            }
            continue; // Skip the fork and wait steps for built-in commands
        }

        // Parse input to get command and arguments
        int arg_count = parse_input(input, args);

        // Fork a new process only if the command exists in PATH
        pid_t pid = fork();

        if (pid == -1) {
            // Error in forking
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            // Execute the command using execvp
            if (execvp(args[0], args) == -1) {
                // Handle command not found
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else {
            // Parent process
            // Wait for the child process to complete
            int status;
            waitpid(pid, &status, 0);

            // Check if the child process exited successfully
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                printf("Error: Command not found\n");
            }
        }
    }

    return 0;
}

