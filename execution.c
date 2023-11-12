#include "shell.h"

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
