#include "shell.h"

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
