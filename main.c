#include "shell.h"

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
