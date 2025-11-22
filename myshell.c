#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include "LineParser.h" // The parser header file

#define MAX_INPUT_SIZE 2048

/**
 * @brief Executes the command specified in pCmdLine.
 * * This function handles the core logic of running a command:
 * 1. Checks if the command is "quit" (internal command).
 * 2. Forks a new process (child).
 * 3. The child process executes the command using execvp.
 * 4. The parent process waits for the child if the command is blocking (not running in background).
 * * @param pCmdLine A pointer to the parsed command line structure.
 */
void execute(cmdLine *pCmdLine) {
    if (pCmdLine == NULL) {
        return;
    }

    // Check for the internal "quit" command
    if (strcmp(pCmdLine->arguments[0], "quit") == 0) {
        // Free the current command line resources before exiting
        freeCmdLines(pCmdLine); 
        exit(0);
    }
    
    // 1. Create a new process (child)
    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        perror("fork failed");
        exit(1);
    } 
    else if (pid == 0) {
        // 2. Child Process: Execute the command
        // Use execvp to search the PATH environment variable for the executable
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
            // execvp only returns if an error occurred.
            // Print an error message and exit the child process abnormally.
            perror("execvp failed");
            // Use _exit() instead of exit() in the child after a fork
            // to avoid issues with flushing buffers and signal handlers.
            _exit(1); 
        }
    } 
    else {
        // 3. Parent Process: Wait for the child if the command is blocking
        // pCmdLine->blocking will be 1 for a foreground process, 0 for a background process (&).
        if (pCmdLine->blocking) {
            // Wait for the specific child process (pid) to change state (e.g., terminate)
            if (waitpid(pid, NULL, 0) == -1) {
                 perror("waitpid failed");
            }
        }
    }
}

int main(int argc, char **argv) {
    char current_path[PATH_MAX];
    char input_buffer[MAX_INPUT_SIZE];
    cmdLine *parsed_line;

    while (1) {
        // 1. Display the prompt (Current Working Directory)
        if (getcwd(current_path, PATH_MAX) != NULL) {
            // Print the path followed by a shell prompt symbol
            printf("%s$ ", current_path);
        } else {
            // In case getcwd fails
            perror("getcwd error");
            printf("$ ");
        }

        // 2. Read a line from the user (stdin)
        if (fgets(input_buffer, MAX_INPUT_SIZE, stdin) == NULL) {
            // Handle EOF (e.g., Ctrl+D)
            break;
        }

        // Remove trailing newline character added by fgets
        input_buffer[strcspn(input_buffer, "\n")] = 0;

        // 3. Parse the input using the provided LineParser library
        parsed_line = parseCmdLines(input_buffer);

        if (parsed_line != NULL) {
            // 4. Execute the command
            execute(parsed_line);

            // 5. Release the allocated cmdLine resources when finished
            freeCmdLines(parsed_line);
        }
    }
    
    printf("Exiting Shell normally.\n");
    return 0;
}