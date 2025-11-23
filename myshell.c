#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <errno.h> // Required for perror
#include "LineParser.h" 

#define MAX_INPUT_SIZE 2048
int g_isDebug = 0; // Global flag for debug mode (Task 1a)

/**
 * @brief Executes the command specified in pCmdLine, including internal commands and process management.
 * @param pCmdLine A pointer to the parsed command line structure.
 */
void execute(cmdLine *pCmdLine) {
    if (pCmdLine == NULL) return;

    // Check for the internal "quit" command
    if (strcmp(pCmdLine->arguments[0], "quit") == 0) {
        // Free the current command line resources before exiting
        freeCmdLines(pCmdLine); 
        exit(0);
    }
    
    // --- Task 1b: Implement the internal "cd" command ---
    // This MUST happen before fork()
    if (strcmp(pCmdLine->arguments[0], "cd") == 0) {
        if (pCmdLine->argCount < 2 || pCmdLine->arguments[1] == NULL) {
            // Check if there is no argument (though parseCmdLines usually provides an empty string)
            fprintf(stderr, "cd: missing argument\n");
        } else {
            // chdir changes the directory of the shell's process itself
            if (chdir(pCmdLine->arguments[1]) == -1) {
                // Print error to stderr if chdir fails (e.g., path doesn't exist)
                perror("chdir failed"); 
            }
        }
        return; // CRITICAL: Exit the execute function without forking!
    }
    // --- End Task 1b ---

    // 1. Create a new process (child)
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        return; 
    } 
    else if (pid == 0) {
        // Child Process
        
        // Task 1a: Print debug info to stderr
        if (g_isDebug) {
            fprintf(stderr, "PID: %d\n", getpid());
            fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
        }
        
        // Execute the command
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
            perror("execvp failed");
            // Task 1a requirement: Use _exit() if execvp fails
            _exit(1); 
        }
    } 
    else {
        // Parent Process

        // Task 1c: Implement waitpid based on blocking status
        if (pCmdLine->blocking) {
            if (waitpid(pid, NULL, 0) == -1) {
                 perror("waitpid failed");
            }
        }
    }
}
int main(int argc, char **argv) {
    // --- Task 1a: Check for -d flag and activate debug mode ---
    // argv[1] is the first argument after the program name
    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        g_isDebug = 1;
        fprintf(stderr, "Debug mode activated.\n");
    }
    // --- End Task 1a Check ---
    
    char current_path[PATH_MAX];
    char input_buffer[MAX_INPUT_SIZE];
    cmdLine *parsed_line;

    while (1) {
        // 1. Display the prompt (Current Working Directory)
        if (getcwd(current_path, PATH_MAX) != NULL) {
            printf("%s$ ", current_path);
        } else {
            // Print to stderr to avoid interfering with stdout
            fprintf(stderr, "Error: Could not retrieve current directory.\n");
            printf("$ ");
        }

        // 2. Read a line from the user (stdin)
        if (fgets(input_buffer, MAX_INPUT_SIZE, stdin) == NULL) {
            // Handle EOF (e.g., Ctrl+D)
            break;
        }

        // Remove trailing newline character added by fgets
        input_buffer[strcspn(input_buffer, "\n")] = 0;

        // 3. Parse the input 
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