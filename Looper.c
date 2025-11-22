#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

/*
 * The custom signal handler for SIGINT, SIGTSTP, and SIGCONT.
 * This handler ensures that after a stop/continue operation, the 
 * handler for the opposite signal is reinstated to maintain control.
 */
void handler(int sig)
{
	// 1. Print the received signal name
	printf("\n[PID %d] Received Signal: %s\n", getpid(), strsignal(sig));
    fflush(stdout); // Ensure the message is printed immediately

    // 2. Handle SIGTSTP (Stop)
    if (sig == SIGTSTP)
    {
        // Reinstate the custom handler for SIGCONT, so we can detect continuation later.
        if (signal(SIGCONT, handler) == SIG_ERR) {
            perror("Error reinstating SIGCONT handler");
        }
        // Set the action for SIGTSTP to default (which stops the process)
        signal(SIGTSTP, SIG_DFL);
    }
    
    // 3. Handle SIGCONT (Continue)
    else if (sig == SIGCONT)
    {
        // Reinstate the custom handler for SIGTSTP, so we can stop again.
        if (signal(SIGTSTP, handler) == SIG_ERR) {
            perror("Error reinstating SIGTSTP handler");
        }
        // Set the action for SIGCONT to default (which continues the process)
        signal(SIGCONT, SIG_DFL); 
    }
    
    // 4. Handle SIGINT (Terminate)
    else if (sig == SIGINT)
    {
        // Set the action for SIGINT to default (which terminates the process)
        signal(SIGINT, SIG_DFL);
    }

	// 5. Re-raise the signal to allow the default system behavior to execute
	raise(sig);
}

int main(int argc, char **argv)
{
	printf("Starting the program with PID: %d\n", getpid());
    fflush(stdout);

    // Set the custom handler for the required signals at startup
	if (signal(SIGINT, handler) == SIG_ERR) {
        perror("Error setting SIGINT handler");
    }
	if (signal(SIGTSTP, handler) == SIG_ERR) {
        perror("Error setting SIGTSTP handler");
    }
	if (signal(SIGCONT, handler) == SIG_ERR) {
        perror("Error setting SIGCONT handler");
    }

	// The looper's infinite loop
	while (1)
	{
		printf("Looper is running...\n");
        fflush(stdout);
		sleep(1);
	}

	return 0;
}