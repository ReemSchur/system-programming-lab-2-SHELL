#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <errno.h> // Required for perror
#include "LineParser.h" 
#include <fcntl.h>   // Required for open() flags (O_WRONLY, O_CREAT, O_TRUNC, O_RDONLY)
#include <signal.h> // **חובה** ל-Task 3 (SIGSTOP, SIGCONT, SIGINT)

#define MAX_INPUT_SIZE 2048
int g_isDebug = 0; // Global flag for debug mode (Task 1a)

// הצהרה על פונקציית העזר (לפני main או execute)
int handle_signal_command(cmdLine *pCmdLine, int sig, const char *cmd_name);
// הצהרה נוספת אם עדיין לא הוספת את הצהרת ה-pipe:
void execute_pipe(cmdLine *pCmdLine);

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
    char *target_dir = NULL;
    
    // 1. Check for the argument
    if (pCmdLine->argCount > 1) {
        target_dir = pCmdLine->arguments[1];
    }

    // 2. Handle missing argument or tilde
    if (target_dir == NULL || strcmp(target_dir, "~") == 0) {
        // If no argument is given, OR the argument is "~", use the HOME environment variable
        target_dir = getenv("HOME");
        
        if (target_dir == NULL) {
            fprintf(stderr, "cd failed: HOME environment variable not set.\n");
            return;
        }
    }
    
    // 3. Attempt to change directory
    if (chdir(target_dir) == -1) {
        // Print error to stderr if chdir fails
        perror("chdir failed"); 
    }
    
    return; // CRITICAL: Exit the execute function without forking!
}
    // --- End Task 1b ---

// --- Task 3: Handle zzzz, kuku, blast using the Helper Function ---
    
    // 1. zzzz <pid> -> SIGSTOP (stop/sleep)
    if (handle_signal_command(pCmdLine, SIGSTOP, "zzzz")) return;

    // 2. kuku <pid> -> SIGCONT (wake up/continue)
    if (handle_signal_command(pCmdLine, SIGCONT, "kuku")) return;
    
    // 3. blast <pid> -> SIGINT (terminate)
    if (handle_signal_command(pCmdLine, SIGINT, "blast")) return;

    // 1. Create a new process (child)
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        return; 
    } 
else if (pid == 0) {
        // Child Process
        if (!pCmdLine->blocking) {
            // Assign a new Process Group ID (PGID) to the background job
            if (setpgid(0, 0) == -1) {
                perror("setpgid failed");
            }

            // Redirect STDIN to /dev/null to prevent the job from blocking the terminal if it requests input
            int dev_null = open("/dev/null", O_RDONLY);
            if (dev_null != -1) {
                dup2(dev_null, STDIN_FILENO);
                close(dev_null);
            }
        }
        // --- Task 2: I/O Redirection Implementation ---
        
        // 1. Output Redirection (>)
        if (pCmdLine->outputRedirect != NULL) {
            // Open the file for writing, create if doesn't exist, and truncate to zero length.
            // Permissions 0644 grant read/write to user, read-only to group/others.
            int fd_out = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out == -1) {
                perror("open output failed");
                _exit(1);
            }
            // Use dup2 to redirect stdout (file descriptor 1) to the new file (fd_out).
            if (dup2(fd_out, STDOUT_FILENO) == -1) { // STDOUT_FILENO is 1
                perror("dup2 output failed failed");
                close(fd_out);
                _exit(1);
            }
            close(fd_out); // Close the temporary descriptor; stdout (1) is now pointing to the file.
        }

        // 2. Input Redirection (<)
        if (pCmdLine->inputRedirect != NULL) {
            // Open the file for reading (O_RDONLY).
            int fd_in = open(pCmdLine->inputRedirect, O_RDONLY);
            if (fd_in == -1) {
                perror("open input failed");
                _exit(1);
            }
            // Use dup2 to redirect stdin (file descriptor 0) to the new file (fd_in).
            if (dup2(fd_in, STDIN_FILENO) == -1) { // STDIN_FILENO is 0
                perror("dup2 input failed failed");
                close(fd_in);
                _exit(1);
            }
            close(fd_in); // Close the temporary descriptor; stdin (0) is now pointing to the file.
        }
        
        // --- End Task 2 I/O Redirection ---

        // Task 1a: Print debug info to stderr (Do this after redirection setup)
        if (g_isDebug) {
            fprintf(stderr, "PID: %d\n", getpid());
            fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
        }
        
        // Execute the command
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
            perror("execvp failed");
            _exit(1); 
        }
    } 
    // ... (rest of the execute function - Parent process continues below) ...
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

// --- Task 3: Helper function to handle signal commands (zzzz, kuku, blast) ---

int handle_signal_command(cmdLine *pCmdLine, int sig, const char *cmd_name) {
    // Check if the current command matches the name (e.g., "zzzz")
    if (strcmp(pCmdLine->arguments[0], cmd_name) == 0) {
        
        // Ensure PID argument exists
        if (pCmdLine->argCount < 2) {
            fprintf(stderr, "%s: Missing process ID.\n", cmd_name);
            return 1; // Handled the command (even if failed)
        }
        
        // Convert the PID argument (string) to an integer
        pid_t target_pid = atoi(pCmdLine->arguments[1]);
        
        // Send the signal using the kill system call
        if (kill(target_pid, sig) == -1) {
            perror("kill failed");
        } else {
            // Optional: print confirmation that the signal was sent
            // fprintf(stderr, "Signal %d sent successfully to PID %d.\n", sig, target_pid);
        }
        return 1; // Command was handled successfully or failed internally
    }
    return 0; // Command was not this signal command
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