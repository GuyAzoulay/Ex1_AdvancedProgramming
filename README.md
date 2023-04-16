# My Own Shell

This project main goal is to implement a simple shell in C/CPP ( we decided in C) language which includes some of the basic shell commands and more:

- ***cd*** - change the current working directory, after the "cd" you can specify the other directory.

- ***echo*** - Simply print what is comming after the word "echo".

- ***!!*** - execute the previous command.

- ***$?*** - print the previous exit status. (of the last command).

- ***prompt*** - Changing the shell prompt to the given argument. 

- ***read*** - Reads an input from the user and store it in a variable. The first line is `read`+ `value` (was implemented using map struct). 
The second line is seperates with enter and contains tha value of the `key`.
For Example :
> read name   
> OR   
> echo $name   
> OR   

- ***>>, 2>, >*** - append output to a file , redirect standard error to a file, redirect standard output to a file.

- ***if else*** -The code allows for conditional execution of commands based on a specified condition. The condition is provided in the command input, followed by the keyword then and the commands to be executed if the condition is true. Optionally, an else part can be included in the command input, followed by the commands to be executed if the condition is false. This enables dynamic decision-making in the code based on the evaluation of the condition.
  `fi` - must be at the end

For Example :   
> if date | grep 14  
> then   
>   echo "Good afternoon"   
> else     
>     echo "Good morning / Good night"   
> fi 

- ***quit*** - Simply exit the shell.

- ***piping*** - execute commands in sequence, passing the output of one command as the input to the next

- ***$*** - Variables can be added to the environment using lines with three parts separated by spaces, following the structure of "$Key = value."  
  Example below : 
> $name =  Guy   
> echo $name   
> Guy

- ***control-C*** - The program will display the message "You typed Control-C!" on the terminal, but the execution of the program will continue without stopping. After printing the message, there will be a line break where you can enter the next command, but the prompt may not be visible.
For Example:
>  hello: ^C
>  You typed Control-C!   

- ***Up and Down arrows*** - By using the up or down arrow keys on your keyboard, you can view your most recent 20 commands in order. After selecting a command using the arrow keys, you need to press "enter" in order to display the selected command.



### How to Run? 

In the terminal we compile this file using 'gcc -o myshell myshell3.c' 



 





