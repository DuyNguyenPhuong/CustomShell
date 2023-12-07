/*
    pipe.c

    Jeff Ondich, 15 April 2004
    Updated 17 February 2022
    Modified by Tanya Amert for Fall 2023

    This program creates one child process and hooks the parent
    and child up via a pipe, like so:

        parent | child

    Then the parent execs "ls" and the child execs "wc -l", so
    we end up running this pipeline:

        ls | wc -l
*/

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main()
{
    int fd[2];

    // Set up the file descriptors for the pipe
    if (pipe(fd) < 0)
    {
        perror("Trouble creating pipe");
        exit(1);
    }

    printf("Pipe file descriptors: fd[0]=%d, fd[1]=%d\n", fd[0], fd[1]);

    // Fork the child process
    int pid = fork();
    if (pid < 0)
    {
        perror("Trouble forking");
        exit(1);
    }

    if (pid != 0)
    {
        /* Parent */
        close(fd[0]);
        if (dup2(fd[1], STDOUT_FILENO) == -1) // set fd[1] to "out"
        {
            perror("Trouble redirecting stdout");
        }
        close(fd[1]);

        // Execute "ls"
        execlp("ls", "ls", NULL);
        perror("execlp in parent failed");
    }
    else
    {
        /* Child */
        close(fd[1]);
        if (dup2(fd[0], STDIN_FILENO) == -1) // set fd[0] to "in"
        {
            perror("Trouble redirecting stdin");
        }
        close(fd[0]);

        // Execute "wc -l" on the result from the parent
        execlp("wc", "wc", "-l", NULL);
        perror("execlp in child failed");
    }

    return 0;
}