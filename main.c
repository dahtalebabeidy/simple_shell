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

char *custom_getline() {
    static char buffer[BUFFER_SIZE];
    static size_t buffer_index = 0;
    static ssize_t bytes_read = 0;

    if (buffer_index >= bytes_read || bytes_read == 0) {
        bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            return NULL;
        }

        buffer_index = 0;
    }

    size_t i = buffer_index;
    while (i < bytes_read && buffer[i] != '\n') {
        i++;
    }

    size_t line_length = i - buffer_index;
    char *line = (char*)malloc(line_length + 1);

    for (size_t j = 0; j < line_length; j++) {
        line[j] = buffer[buffer_index + j];
    }

    line[line_length] = '\0';

    buffer_index = i + 1;

    return line;
}

void display_prompt() {
    printf("simple_shell$ ");
    fflush(stdout);
}

int parse_input(char *input, char *commands[]) {
    int i = 0;
    char *token = strtok(input, ";");

    while (token != NULL && i < MAX_ARGS - 1) {
        while (*token == ' ' || *token == '\t') {
            token++;
        }

        char *end = token + strlen(token) - 1;
        while (end > token && (*end == ' ' || *end == '\t')) {
            end--;
        }
        end[1] = '\0';

        commands[i++] = token;
        token = strtok(NULL, ";");
    }

    commands[i] = NULL;
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
                free(*env);
                *env = malloc(strlen(name) + strlen(value) + 2);
                sprintf(*env, "%s=%s", name, value);
                return 0;
            }
        }
        env++;
    }

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
    const char *new_dir;

    if (dir == NULL || strcmp(dir, "") == 0) {
        new_dir = getenv("HOME");
    } else if (strcmp(dir, "-") == 0) {
        new_dir = getenv("OLDPWD");
        if (new_dir == NULL) {
            fprintf(stderr, "cd: OLDPWD not set\n");
            return -1;
        }
    } else {
        new_dir = dir;
    }
    char current_dir[BUFFER_SIZE];
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("getcwd");
        return -1;
    }

    if (setenv("OLDPWD", current_dir, 1) != 0) {
        perror("setenv");
        return -1;
    }

    if (chdir(new_dir) != 0) {
        perror("chdir");
        return -1;
    }

    if (setenv("PWD", new_dir, 1) != 0) {
        perror("setenv");
        return -1;
    }

    return 0;
}

int execute_command(char *command) {
    char *args[MAX_ARGS];
    int arg_count = parse_input(command, args);

    int success = 1;

    for (int i = 0; i < arg_count; i++) {
        char *arg = args[i];

        if (strcmp(arg, "&&") == 0) {
            if (success) {
                continue;
            } else {
                break;
            }
        } else if (strcmp(arg, "||") == 0) {
            if (!success) {
                continue;
            } else {
                break;
            }
        }

        if (strncmp(arg, "exit", 4) == 0) {
            int status_code = 0;
            if (strlen(arg) > 4) {
                status_code = atoi(arg + 5);
            }

            printf("Exiting shell with status code: %d\n", status_code);
            exit(status_code);
        } else if (strcmp(arg, "env") == 0) {
            print_environment();
        } else if (strncmp(arg, "setenv", 6) == 0) {
            char *name = strtok(arg + 6, " ");
            char *value = strtok(NULL, " ");
            if (name != NULL && value != NULL) {
                custom_setenv(name, value, 1);
            } else {
                fprintf(stderr, "setenv: Invalid syntax\n");
            }
        } else if (strncmp(arg, "unsetenv", 8) == 0) {
            char *name = strtok(arg + 8, " ");
            if (name != NULL) {
                custom_unsetenv(name);
            } else {
                fprintf(stderr, "unsetenv: Invalid syntax\n");
            }
        } else if (strncmp(arg, "cd", 2) == 0) {
            char *dir = strtok(arg + 2, " ");
            if (dir != NULL) {
                custom_cd(dir);
            } else {
                custom_cd(NULL);
            }
        } else {
            pid_t pid = fork();

            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                if (execvp(arg, args) == -1) {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            } else {
                int status;
                waitpid(pid, &status, 0);

                if (WIFEXITED(status)) {
                    success = (WEXITSTATUS(status) == 0);
                } else {
                    success = 0;
                }
            }
        }
    }

    return 0;
}
          

int main() {
    char input[MAX_INPUT_SIZE];

    while (1) {
        display_prompt();
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';
        execute_command(input);
    }

    return 0;
}
