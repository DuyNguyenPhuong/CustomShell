#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_args(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    // argv is an array of character pointers; here are their values
    print_args(argc + 1, argv);

    // Can you build your own argv and pass it to a function? Yes!

    // Method 1: malloc your pointers, stick the pointers in argv
    int my_argc = 4;
    char *my_argv[my_argc];
    for (int k = 0; k < my_argc; k++)
    {
        my_argv[k] = malloc(10);
        if (my_argv[k] != NULL)
        {
            strcpy(my_argv[k], "moose");
        }
        else
        {
            fprintf(stderr, "Bad malloc, so sad. I'm outta here.\n");
            exit(1);
        }
    }

    print_args(my_argc, my_argv);

    for (int k = 0; k < my_argc; k++)
    {
        free(my_argv[k]); // we called malloc, so let's clean up!
    }

    // Method 2: Initialize one char buffer with nulls inside it, and
    //    point argv[k] to successive substrings
    char *arguments = "emu\0goat\0newt\0";
    my_argc = 3;
    my_argv[0] = &arguments[0];
    my_argv[1] = &arguments[4];
    my_argv[2] = &arguments[9];
    print_args(my_argc, my_argv);

    // Want something wackier that could be helpful?  Search for "strtok example"

    return 0;
}

void print_args(int argc, char *argv[])
{
    printf("==== Some args for you ====\n");
    for (int k = 0; k < argc; k++)
    {
        if (argv[k] == NULL)
        {
            printf("argv[%d]: %p, %s\n", k, argv[k], "NULL");
        }
        else
        {
            printf("argv[%d]: %p, %s\n", k, argv[k], argv[k]);
        }
    }
    printf("\n");
}