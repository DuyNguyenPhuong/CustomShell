/*
    This program gives a simple example of execvp.
    See also exectest.c.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    pid_t pid = fork();

    if (pid == 0)
    {
        /* Child */
        printf("Process %d is about to execute some command.\n", getpid());
        fflush(stdout);

        // Prep arguments
        char *arguments = "ls\0-l\0-a\0";
        int my_argc = 4;
        char *my_argv[my_argc];
        my_argv[0] = &arguments[0];
        my_argv[1] = &arguments[3];
        my_argv[2] = &arguments[6];
        my_argv[3] = NULL;

        // Execute the program
        execvp(my_argv[0], my_argv);

        // Check for errors
        perror("exec failed");
        fflush(stdout);
    }
    else
    {
        /* Parent */
        printf("Parent just created child, ID %d\n", pid);
        fflush(stdout);

        // Wait for the child to finish execution
        int status;
        pid = wait(&status);
        printf("My child ID %d just exited with status %d\n", pid, WEXITSTATUS(status));
        fflush(stdout);
    }

    return 0;
}