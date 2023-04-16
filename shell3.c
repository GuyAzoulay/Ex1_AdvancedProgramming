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
#define MAX_ARGS 10
#define MAXHISTORY 20


#define MAX_SIZE 100

// Define a struct for key-value pairs
typedef struct {
    char* key;
    char* value;
} KeyValuePair;

// Define a struct for the map
typedef struct {
    KeyValuePair pairs[MAX_SIZE];
    int size;
} Map;

// Function to add a key-value pair to the map
void add(Map* map, const char* key, const char* value) {
    if (map->size < MAX_SIZE) {
        KeyValuePair pair;
        pair.key = strdup(key);
        pair.value = strdup(value);
        map->pairs[map->size++] = pair;
    } else {
        printf("Map is full. Unable to add more entries.\n");
    }
}

// Function to get the value associated with a given key
const char* get(const Map* map, const char* key) {
    for (int i = 0; i < map->size; ++i) {
        if (strcmp(map->pairs[i].key, key) == 0) {
            return map->pairs[i].value;
        }
    }
    return NULL; // Return NULL if key is not found
}

// Function to free memory allocated by the map
void freeMap(Map* map) {
    for (int i = 0; i < map->size; ++i) {
        free(map->pairs[i].key);
        free(map->pairs[i].value);
    }
}

int main() {
    char command[1024];
    char temp_command[1024];
    char *command_list[MAXHISTORY];
    char *token;
    int i;
    char *outfile;
    int fd, amper, redirect, piping, retid, status, argc1, commandIndex = 0;
    int fildes[2];
    char *argv1[10], *argv2[10];
    char prompt[64] = "hello";
    int last_return_value = 0;
    bool last_command_exists = false;
    char command_history[1024] = "";
    int arrow = 0;
    Map map;
    signal(SIGINT, handle_sigint);

    while (1)
    {

        printf("%s: ", prompt);
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';
        piping = 0;
        strcpy(temp_command, command);
        arrow = command[0];

        if (strstr(command, "prompt = ") == command) {
            char new_prompt[64];
            sscanf(command, "prompt = %s", new_prompt);
            strcpy(prompt, new_prompt);
            continue;
        }
        /* Handle 'cd' command */
        if (strncmp(command, "cd", 2) == 0) {
            /* Check if the command is 'cd ..' */
            if (strcmp(command, "cd ..") == 0) {
                chdir("..");
                continue;
            }
            /* Parse the subdirectory name for 'cd [subdir]' */
            token = strtok(command + 3, " ");
            if (token == NULL) {
                fprintf(stderr, "cd: missing operand\n");
                continue;
            }
            chdir(token);
            continue;
        }

        /* Handle 'echo $?' command */
        if (strcmp(command, "echo $?") == 0) {
            printf("%d\n", last_return_value);
            continue;
        }
        command_list[commandIndex] = command;
        /* Arrow up/down command */
            while (arrow == '\033')
            {
               
                printf("\033[1A"); // line up
                printf("\x1b[2K"); // delete line
                switch (command[2])
                {
                case 'A':
                    // code for arrow up
                    //printf("here2\n");
                    memset(command, 0, sizeof(command));
                    commandIndex--;
                    //commandIndex = commandIndex % MAXHISTORY;
                    printf("commandIndex: %d\n", commandIndex);
                    strcpy(command, command_list[commandIndex]);
                    //printf("command: %s\n", command_list[commandIndex]);
                    printf("%s: ", prompt);
                    printf("%s \n", command_list[commandIndex]);
                    break;

                case 'B':
                    // code for arrow down
                    memset(command, 0, sizeof(command));
                    commandIndex++;
                    commandIndex = commandIndex % MAXHISTORY;
                    strcpy(command, command_list[commandIndex]);
                     printf("%s: %s\n", prompt, command);
                    break;
                }
                char tempCommand[1024];
                fgets(tempCommand, 1024, stdin);
                tempCommand[strlen(tempCommand) - 1] = '\0';
                arrow = tempCommand[0];
                piping = 0;
                strcat(command, tempCommand);
                if (arrow != '\033')
                {
                    break;
                }
                else
                {
                    strcpy(command, tempCommand);
                }
            }
            command_list[commandIndex] = command;
            
            commandIndex++;
            commandIndex = commandIndex % MAXHISTORY;



        // Handle '!!' command
        if (strcmp(command, "!!") == 0) {
            if (!last_command_exists) {
                printf("No previous command\n");
                continue;
            }
            printf("%s\n", command_history);
            system(command_history);
            continue;
        }
        // Save command to history
        strcpy(command_history, command);
        last_command_exists = true;


         if (strcmp(command, "quit") == 0){
            break;
         }
        
        i = 0;
        token = strtok (command," ");
        while (token != NULL)
        {
            argv1[i] = token;
            token = strtok (NULL, " ");
            i++;
            if (token && ! strcmp(token, "|")) {
                piping = 1;
                break;
            }
        }
        argv1[i] = NULL;
        argc1 = i;

        /* Is command empty */
        if (argv1[0] == NULL)
            continue;

        /* Does command contain pipe */
        if (piping) {
            i = 0;
            while (token!= NULL)
            {
                token = strtok (NULL, " ");
                argv2[i] = token;
                i++;
            }
            argv2[i] = NULL;
        }

        /* Does command line end with & */ 
        if (! strcmp(argv1[argc1 - 1], "&")) {
            amper = 1;
            argv1[argc1 - 1] = NULL;
        }
        else 
            amper = 0; 

        if (argc1 > 1 && ! strcmp(argv1[argc1 - 2], ">")) {
            redirect = 1;
            argv1[argc1 - 2] = NULL;
            outfile = argv1[argc1 - 1];
        }
        else if (argc1 > 1 && ! strcmp(argv1[argc1 - 2], "2>")) {
            redirect = 2;
            argv1[argc1 - 2] = NULL;
            outfile = argv1[argc1 - 1];
        }
        else if (argc1 > 1 && ! strcmp(argv1[argc1 - 2], ">>")) {
            redirect = 3;
            argv1[argc1 - 2] = NULL;
            outfile = argv1[argc1 - 1];
        }
        else 
            redirect = 0; 




        /* set variable command */
            if (argv1[0][0] == '$' && !strcmp(argv1[1], "="))
            {

                char *s = argv1[0] + 1;
                add(&map, s, argv1[2]);
               
                continue;
            }

            /* echo command */
            if (!strcmp(argv1[0], "echo"))
            {
                // check if the argument is empty
                if (argv1[1] == NULL)
                {
                    
                    printf("echo: expected argument to \"echo\"\n");
                }
                /* echo $? command */
                if (!strcmp(argv1[1], "$?"))
                {
                    
                    printf("%d\n", last_return_value);
                    
                    continue;
                }

                for (int j = 1; j < i; j++)
                {
                    // check if the argument is a variable
                    if (argv1[j][0] == '$')
                    {
                        // get the variable name from the map and print it out
                        char *var = argv1[j] + 1;
                        if (get(&map, var) != NULL)
                        {
                            
                            printf("%s ", get(&map, var));
                        }
                        else
                        {
                            
                            printf("echo: %s: Undefined variable.\n", var);
                        }
                    }
                    // print out the rest of the command line except for the echo command
                    else
                    {
                        
                        printf("%s ", argv1[j]);
                    }
                }
                printf("\n");
                continue;
            }    

         /* IF/ELSE command */
            if (command[0] == 'i' && command[1] == 'f')
            {
                printf("here 1\n");
                // take all the command ecxept the first argument
                strcpy(command, temp_command + 2);
                char then[1024];
                fgets(then, 1024, stdin);
                printf("here 2\n");
                if (then[0] == 't' && then[1] == 'h' && then[2] == 'e' && then[3] == 'n')
                {
                      
                    char ThenCommand[1024];
                    fgets(ThenCommand, 1024, stdin);
                    char NextCommand[1024];
                    fgets(NextCommand, 1024, stdin);
                    if (NextCommand[0] == 'f' && NextCommand[1] == 'i')
                    {
                        if (!system(command))
                        {
                            // check if if statement is true, and execute the command
                            strcpy(command, " ");
                            system(ThenCommand);
                        }
                        else
                        {
                            strcpy(command, " ");
                            continue;
                        }
                    }
                    // check if there is an else statement
                    else if (NextCommand[0] == 'e' && NextCommand[1] == 'l' && NextCommand[2] == 's' && NextCommand[3] == 'e')
                    {
                        char elseCommand[1024];
                        fgets(elseCommand, 1024, stdin);
                        char fi[1024];
                        fgets(fi, 1024, stdin);
                        if (fi == "fi")
                        {
                            // check if if statement is true, and execute the command
                            if (!system(command))
                            {
                                strcpy(command, " ");
                                system(ThenCommand);
                            }
                            else
                            {
                                // check if if statement is false, and execute the command
                                strcpy(command, " ");
                                system(elseCommand);
                            }
                        }
                    }
                }
                else
                {
                    printf ("syntax error\n");
                    continue;
                }
            }
            /* read command */
            if (!strcmp(argv1[0], "read"))
            {
                // check if the argument is empty
                if (argv1[1] == NULL)
                {
                    
                    printf("read: expected argument to \"read\"\n");
                }
                else
                {
                    char *s;
                    fgets(s, 1024, stdin);
                    add(&map, argv1[1], s);
                    //cin.ignore();
                }
                continue;
            }

        /* for commands not part of the shell command language */ 

        if (fork() == 0) { 
            /* redirection of IO ? */
            if (redirect) {
                if (redirect == 1) {
                    fd = creat(outfile, 0660); 
                    close (STDOUT_FILENO) ; 
                    dup(fd); 
                    close(fd); 
                    /* stdout is now redirected */
                } else if (redirect == 2) {
                    fd = creat(outfile, 0660); 
                    close (STDERR_FILENO) ; 
                    dup(fd); 
                    close(fd); 
                    /* stderr is now redirected */
               } else if (redirect == 3) {
                fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0660); 
                close (STDOUT_FILENO) ; 
                dup(fd); 
                close(fd); 
                /* stdout is now appended */
            }
        } 

        if (piping) {
            pipe (fildes);
            if (fork() == 0) { 
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

void handle_sigint(int sig) {
    char* str = "\nYou typed Control-C!\n";
    printf(str);
    
  
}
