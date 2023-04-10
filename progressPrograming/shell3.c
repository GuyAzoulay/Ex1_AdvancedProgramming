#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <stdbool.h>
#include <signal.h>

void handle_sigint(int sig);
#define MAX_ARGS 10;
int main()
{
    char command[1024];
    char *token;
    int i;
    char *outfile;
    int fd, amper, redirect, piping, retid, status, argc1;
    int fildes[2];
    char *argv1[10], *argv2[10];
    char prompt[64] = "hello";
    int last_return_value = 0;
    bool last_command_exists = false;
    char command_history[1024] = "";
    signal(SIGINT, handle_sigint); // Q.8

    while (1)
    {

        printf("%s: ", prompt);
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';
        piping = 0;

        /* Handle 'prompt = [new_prompt]' command
            Q.2                                     */
        if (strstr(command, "prompt = ") == command)
        {
            char new_prompt[64];
            sscanf(command, "prompt = %s", new_prompt);
            strcpy(prompt, new_prompt);
            continue;
        }
        /* Handle 'cd' command
            Q.5               */
        if (strncmp(command, "cd", 2) == 0)
        {
            /* Check if the command is 'cd ..' */
            if (strcmp(command, "cd ..") == 0)
            {
                chdir("..");
                continue;
            }
            /* Parse the subdirectory name for 'cd [subdir]' */
            token = strtok(command + 3, " ");
            if (token == NULL)
            {
                fprintf(stderr, "cd: missing operand\n");
                continue;
            }
            chdir(token);
            continue;
        }

        /* Handle 'echo $?' command
           Q.4                     */
        if (strcmp(command, "echo $?") == 0)
        {
            printf("%d\n", last_return_value);
            continue;
        }

        // Handle '!!' command
        // Q.6
        if (strcmp(command, "!!") == 0)
        {
            if (!last_command_exists)
            {
                printf("No previous command\n");
                continue;
            }
            printf("%s\n", command_history);
            system(command_history);
            continue;
        }

        /* Handle 'quit' command
            Q.7                 */
        if (strcmp(command, "quit") == 0)
        {
            break;
        }
        // // for multi pipes
        // char **pipes[MAX_ARGS];
        // int num_pipes = 0;
        // int j = 0;

        /* parse command line */
        i = 0;
        token = strtok(command, " ");
        while (token != NULL)
        {
            argv1[i] = token;
            token = strtok(NULL, " ");
            i++;
            if (token && !strcmp(token, "|"))
            {
                piping = 1;
                break;
            }
        }
        argv1[i] = NULL;
        argc1 = i;

        /* Is command empty */
        if (argv1[0] == NULL)
            continue;
        else
        {
            // Save command to history
            strcpy(command_history, command);
            last_command_exists = true;
        }

        /* Does command contain pipe */
        if (piping)
        {
            i = 0;
            while (token != NULL)
            {
                token = strtok(NULL, " ");
                argv2[i] = token;
                i++;
            }
            argv2[i] = NULL;
        }

        /* Does command line end with & */
        if (!strcmp(argv1[argc1 - 1], "&"))
        {
            amper = 1;
            argv1[argc1 - 1] = NULL;
        }
        else
            amper = 0;

        /* Q.1 */
        if (argc1 > 1 && !strcmp(argv1[argc1 - 2], ">"))
        {
            redirect = 1;
            argv1[argc1 - 2] = NULL;
            outfile = argv1[argc1 - 1];
        }
        else if (argc1 > 1 && !strcmp(argv1[argc1 - 2], "2>"))
        {
            redirect = 2;
            argv1[argc1 - 2] = NULL;
            outfile = argv1[argc1 - 1];
        }
        else if (argc1 > 1 && !strcmp(argv1[argc1 - 2], ">>"))
        {
            redirect = 3;
            argv1[argc1 - 2] = NULL;
            outfile = argv1[argc1 - 1];
        }
        else
            redirect = 0;

        /* for commands not part of the shell command language */

        if (fork() == 0)
        {
            /* redirection of IO ? */
            if (redirect)
            {
                if (redirect == 1)
                {
                    fd = creat(outfile, 0660);
                    close(STDOUT_FILENO);
                    dup(fd);
                    close(fd);
                    /* stdout is now redirected */
                }
                else if (redirect == 2)
                {
                    fd = creat(outfile, 0660);
                    close(STDERR_FILENO);
                    dup(fd);
                    close(fd);
                    /* stderr is now redirected */
                }
                else if (redirect == 3)
                {
                    fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0660);
                    close(STDOUT_FILENO);
                    dup(fd);
                    close(fd);
                    /* stdout is now appended */
                }
            }

            if (piping)
            {
                pipe(fildes);
                if (fork() == 0)
                {
                    /* first component of command line */
                    close(STDOUT_FILENO);
                    dup(fildes[1]);
                    close(fildes[1]);
                    close(fildes[0]);
                    /* stdout now goes to pipe */
                    /* child process does command */
                    execvp(argv1[0], argv1);
                }
                /* 2nd command component of command line */
                close(STDIN_FILENO);
                dup(fildes[0]);
                close(fildes[0]);
                close(fildes[1]);
                /* standard input now comes from pipe */
                execvp(argv2[0], argv2);
            }
            else
                /* execute the command and store the return value */
                last_return_value = execvp(argv1[0], argv1);
        }
        /* parent continues over here... */
        /* waits for child to exit if required */
        if (amper == 0)
            retid = wait(&status);
    }
}

void handle_sigint(int sig)
{
    char *str = "\nYou typed Control-C!\n";
    printf(str);

    // fflush(stdout);
}
