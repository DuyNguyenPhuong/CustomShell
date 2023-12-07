/*
    Authur: Duy Nguyen
    Collaborators: I made this file based on the file shell208.c,
    argv.c, exectest_with_args.c, redirect.c, pipe.c, signaltest.c
    made by Jeff Ondich and modified by Tanya Amert
*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

// The command buffer will need to have room to hold the
// command, the \n at the end of the command, and the \0.
// That's why the maximum command size is 2 less than the
// command buffer size.
#define COMMAND_BUFFER_SIZE 102
#define MAX_COMMAND_SIZE COMMAND_BUFFER_SIZE - 2

// Return values for get_command
#define COMMAND_INPUT_SUCCEEDED 0
#define COMMAND_INPUT_FAILED 1
#define COMMAND_END_OF_FILE 2
#define COMMAND_TOO_LONG 3

#define MAX_ARGS 128
#define MAX_PIPES 10

/*
    Retrieves the next line of input from stdin (where, typically, the user
    has typed a command) and stores it in command_buffer.

    The newline character (\n, ASCII 10) character at the end of the input
    line will be read from stdin but not stored in command_buffer.

    The input stored in command_buffer will be \0-terminated.

    Returns:
        COMMAND_TOO_LONG if the number of chars in the input line
            (including the \n), is greater than or equal to buffer_size
        COMMAND_INPUT_FAILED if the input operation fails with an error
        COMMAND_END_OF_FILE if the input operation fails with feof(stdin) == true
        COMMAND_INPUT_SUCCEEDED otherwise

    Preconditions:
        - buffer_size > 0
        - command_buffer != NULL
        - command_buffer points to a buffer large enough for at least buffer_size chars
*/
int get_command(char *command_buffer, int buffer_size)
{
    assert(buffer_size > 0);
    assert(command_buffer != NULL);

    if (fgets(command_buffer, buffer_size, stdin) == NULL)
    {
        if (feof(stdin))
        {
            return COMMAND_END_OF_FILE;
        }
        else
        {
            return COMMAND_INPUT_FAILED;
        }
    }

    int command_length = strlen(command_buffer);
    if (command_buffer[command_length - 1] != '\n')
    {
        // If we get here, the input line hasn't been fully read yet.
        // We need to read the rest of the input line so the unread portion
        // of the line doesn't corrupt the next command the user types.
        // Note that we won't store what we read, now, though, as the
        // user hasn't entered a valid command (because it's too long).
        char ch = getchar();
        while (ch != '\n' && ch != EOF)
        {
            ch = getchar();
        }

        return COMMAND_TOO_LONG;
    }

    // remove the newline character
    command_buffer[command_length - 1] = '\0';
    return COMMAND_INPUT_SUCCEEDED;
}

// Helper function to print help information.
void print_help()
{
    printf("shell208 Do you need help:\n");
    printf("Type the commands and hit enter.\n");
    printf("Here are some commands commands:\n");
    printf("help\tShow this help message\n");
    printf("exit\tExit the shell\n");
    printf("ls\tList directory contents\n");
    printf("pwd\tPrint the working directory\n");
    printf("echo\tDisplay a line of text\n");
    printf("ls -l\tList directory contents in long format\n");
    printf("Any command followed by > and a filename redirects the output to the file.\n");
}

// Parses a command line into its arguments and identifies any input or output redirection
int parse_command_with_input_output(char *command_line, char *args[], char **input_file, char **output_file)
{
    // Argument count
    int argc = 0;
    // Tokenize the command line by spaces
    char *token = strtok(command_line, " ");
    while (token != NULL)
    {
        // If we saw a stdout sign
        if (strcmp(token, ">") == 0)
        {
            // Get the next token, which should be the file name
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                fprintf(stderr, "Syntax error: Expected output file name after '>'\n");
                return -1;
            }
            *output_file = token;
        }
        // If we saw a stdin sign
        else if (strcmp(token, "<") == 0)
        {
            // Get the next token, which should be the file name
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                fprintf(stderr, "Syntax error: Expected input file name after '<'\n");
                return -1;
            }
            *input_file = token;
        }
        else
        {
            args[argc++] = token;
        }
        token = strtok(NULL, " ");
    }
    // Null-terminate the list of arguments
    args[argc] = NULL;
    // Return number of arguments
    return argc;
}

// stdout redirection
int setup_stdout_redirection(const char *redirect_file)
{
    int fd = open(redirect_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("Trouble opening file");
        return -1; // Return an error code
    }
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        perror("dup2 failed");
        close(fd);
        return -1; // Return an error
    }
    close(fd);
    return 0; // Success
}

// stdin redirection
int setup_stdin_redirection(const char *input_file)
{
    int fd = open(input_file, O_RDONLY);
    if (fd == -1)
    {
        perror("Trouble opening input file");
        return -1;
    }
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        perror("dup2 failed for input");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

// Executes the command using fork and execvp
// Handling output and input redirection if specified.
int execute_command_with_input_output(char *args[], char *input_file, char *output_file)
{
    pid_t pid = fork();

    // Child Process
    if (pid == 0)
    {
        if (input_file != NULL && setup_stdin_redirection(input_file) == -1)
        {
            perror("Trouble opening file");
            exit(1);
        }

        if (output_file != NULL && setup_stdout_redirection(output_file) == -1)
        {
            perror("Trouble opening file");
            exit(1);
        }

        if (strcmp(args[0], "help") == 0)
        {
            // Handle 'help' command in child process
            print_help();
            exit(EXIT_SUCCESS);
        }
        else
        {
            // Execute other commands
            // Replaces the child process with the new process specified by args.
            execvp(args[0], args);
            // If execvp returns, it must have failed.
            perror("execvp failed");
            exit(1);
        }
    }
    else if (pid > 0)
    {
        /* Parent */
        // Wait for the child to finish execution
        int status;
        pid = wait(&status);
        fflush(stdout);
    }
    else
    {
        // Fork failed.
        perror("fork failed");
        exit(1);
    }

    return 0;
}

// Parses a command line into a 2D commands array
// where each row is a subcommand
// and it skip file name if it encounters stdout or stdin redirection
int parse_commands_with_pipes(char *command_line, char *commands[MAX_PIPES][MAX_ARGS])
{
    int command_count = 0;
    int arg_count = 0;

    // char command_line[COMMAND_BUFFER_SIZE];
    // // Make a copy of the command line to avoid modifying the original
    // strncpy(command_line, command_line_orginal, COMMAND_BUFFER_SIZE);
    command_line[COMMAND_BUFFER_SIZE - 1] = '\0';

    char *token = strtok(command_line, " ");
    while (token != NULL)
    {
        if (strcmp(token, "|") == 0)
        {
            commands[command_count][arg_count] = NULL; // End of command
            command_count++;

            arg_count = 0; // Reset argument count for next command
        }
        else if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0)
        {
            // Skip redirections and the subsequent file names
            strtok(NULL, " "); // Skip the file name
        }
        else
        {
            commands[command_count][arg_count++] = token;
        }
        token = strtok(NULL, " ");
    }
    commands[command_count][arg_count] = NULL; // NULL-terminate last command

    return command_count + 1; // Number of commands
}

void execute_piped_commands(char *commands[MAX_PIPES][MAX_ARGS], int num_commands, char *input_file, char *output_file)
{
    // Create the necessary pipes for the pipeline
    int pipe_fds[2 * (num_commands - 1)];
    int i;

    // Create pipes
    for (i = 0; i < num_commands - 1; i++)
    {
        if (pipe(pipe_fds + i * 2) < 0)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Execute each command
    for (i = 0; i < num_commands; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        { // Child
            if (i == 0 && input_file)
            { // First command with input redirection
                if (setup_stdin_redirection(input_file) == -1)
                    exit(EXIT_FAILURE);
            }
            if (i == num_commands - 1 && output_file)
            { // Last command with output redirection
                if (setup_stdout_redirection(output_file) == -1)
                    exit(EXIT_FAILURE);
            }
            if (i != 0)
            { // Not the first command, get input from previous pipe
                dup2(pipe_fds[(i - 1) * 2], STDIN_FILENO);
            }
            if (i != num_commands - 1)
            { // Not the last command, output to next pipe
                dup2(pipe_fds[i * 2 + 1], STDOUT_FILENO);
            }
            // Close all pipe fds
            for (int j = 0; j < 2 * (num_commands - 1); j++)
            {
                close(pipe_fds[j]);
            }
            execvp(commands[i][0], commands[i]);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        else if (pid < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Parent closes all pipe fds
    for (i = 0; i < 2 * (num_commands - 1); i++)
    {
        close(pipe_fds[i]);
    }

    // Parent waits for all child processes
    for (i = 0; i < num_commands; i++)
    {
        wait(NULL);
    }
}

// Print ^C symbol and info string
void interrupt_handler(int sig)
{
    fprintf(stderr, "^CSorry, but I just don't want to quit.\n");
    fflush(stderr);
}

int main()
{
    const char *prompt = "shell208> ";
    char command_line[COMMAND_BUFFER_SIZE];

    char *args[MAX_ARGS];

    if (signal(SIGINT, interrupt_handler) != SIG_DFL)
    {
        fprintf(stderr, "I'm confused.\n");
    }

    // The main infinite loop
    while (1)
    {
        char *input_file = NULL;
        char *output_file = NULL;
        char *commands[MAX_PIPES][MAX_ARGS];
        printf("%s", prompt);
        fflush(stdout);

        // Resetting the argument arrays
        memset(args, 0, sizeof(args));
        memset(command_line, 0, sizeof(command_line));
        memset(commands, 0, sizeof(commands));

        int result = get_command(command_line, COMMAND_BUFFER_SIZE);
        if (result == COMMAND_END_OF_FILE)
        {
            // stdin has reached EOF, so it's time to be done; this often happens
            // when the user hits Ctrl-D
            break;
        }
        else if (result == COMMAND_INPUT_FAILED)
        {
            fprintf(stderr, "There was a problem reading your command. Please try again.\n");
            // we could try to analyze the error using ferror and respond in different
            // ways depending on the error, but instead, let's just bail
            break;
        }
        else if (result == COMMAND_TOO_LONG)
        {
            fprintf(stderr, "Commands are limited to length %d. Please try again.\n", MAX_COMMAND_SIZE);
        }
        else
        {
            // Handle 'exit' command.
            if (strcmp(command_line, "exit") == 0)
            {
                break;
            }

            // Make a copy of the command line to avoid modifying the original
            char command_line_copy[COMMAND_BUFFER_SIZE];
            strncpy(command_line_copy, command_line, COMMAND_BUFFER_SIZE);

            int num_commands = parse_commands_with_pipes(command_line, commands);

            // Parse the command line into arguments and check for redirection.
            int argc = parse_command_with_input_output(command_line_copy, args, &input_file, &output_file);
            if (argc <= 0)
            {
                fprintf(stderr, "Error: Invalid command syntax.\n");
                continue; // Skip execution on parsing error.
            }

            if (num_commands == 1) // If there is no pipe
            {
                execute_command_with_input_output(args, input_file, output_file);
            }
            else // If there is at least a pipe
            {
                execute_piped_commands(commands, num_commands, input_file, output_file);
            }
        }
    }
    return 0;
}