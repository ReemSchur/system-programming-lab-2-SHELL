# Define the compiler and flags
CC = gcc
# CFLAGS: -Wall enables common warnings, -g includes debug information
# NOTE: The '-m32' flag is included here to match common lab requirements 
# for 32-bit compilation, but can be removed if your environment defaults to 64-bit 
# and the TA does not require 32-bit.
CFLAGS = -Wall -g -m32

# External LineParser files (used by myshell)
LINEPARSER_OBJS = LineParser.o

# --- Main Targets ---

# Default target: builds both myshell and looper
all: myshell looper

# Target 1: Compiling the myshell executable (Task 0a)
myshell: myshell.o $(LINEPARSER_OBJS)
	$(CC) $(CFLAGS) -o myshell myshell.o $(LINEPARSER_OBJS)

# Target 2: Compiling the looper executable (Task 0b)
looper: Looper.o
	$(CC) $(CFLAGS) -o looper Looper.o

# --- Object File Rules ---

# Rule for myshell object file
myshell.o: myshell.c LineParser.h
	$(CC) $(CFLAGS) -c myshell.c

# Rule for LineParser object file
LineParser.o: LineParser.c LineParser.h
	$(CC) $(CFLAGS) -c LineParser.c

# Rule for Looper object file
Looper.o: Looper.c
	$(CC) $(CFLAGS) -c Looper.c

# --- Cleanup Rule ---

# Target to clean up compiled files, executables, and object files
clean:
	rm -f *.o myshell looper