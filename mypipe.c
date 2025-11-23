#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1
#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
    int pipefd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s \"<message>\"\n", argv[0]);
        return 1;
    }

    // 1. Create the pipe
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return 1;
    }

    // 2. Fork the process
    pid = fork();

    if (pid == -1) {
        perror("fork failed");
        close(pipefd[READ_END]);
        close(pipefd[WRITE_END]);
        return 1;
    }

    if (pid == 0) {
        // Child Process: Writes the message
        close(pipefd[READ_END]); // Close the unused read end

        // Write the message (argv[1]) to the pipe
        if (write(pipefd[WRITE_END], argv[1], strlen(argv[1]) + 1) == -1) {
            perror("child write failed");
            _exit(1);
        }

        close(pipefd[WRITE_END]); // Close the write end
        _exit(0);
    } else {
        // Parent Process: Reads the message

        close(pipefd[WRITE_END]); // Close the unused write end

        // Read the message from the pipe
        ssize_t bytes_read = read(pipefd[READ_END], buffer, BUFFER_SIZE);
        
        if (bytes_read > 0) {
            printf("Parent received message: %s\n", buffer);
        } else if (bytes_read == -1) {
            perror("parent read failed");
        }

        close(pipefd[READ_END]); // Close the read end
        waitpid(pid, NULL, 0); // Wait for the child to finish
    }

    return 0;
}