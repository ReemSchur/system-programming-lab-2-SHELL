# Define the compiler and flags
CC = gcc
# -Wall enables all common warnings, -g includes debug information
CFLAGS = -Wall -g

# The main target: Compiling the myshell executable
# Dependencies: myshell.c and LineParser.c
myshell: myshell.c LineParser.c
	$(CC) $(CFLAGS) myshell.c LineParser.c -o myshell

# Target for Task 0b: Compiling the looper executable
looper: looper.c
	$(CC) $(CFLAGS) looper.c -o looper

# A target to clean up compiled files and executables
clean:
	rm -f myshell looper *.o