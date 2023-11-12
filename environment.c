#include "shell.h"

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
