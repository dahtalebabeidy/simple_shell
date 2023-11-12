#include "shell.h"

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
