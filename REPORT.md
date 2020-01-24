---
ECS150 Project #1 Report
---
**General Flow** The general flow of our shell is as follows:

 1. The program is executed and initially a struct used to contain details of all the processes named *Task* is initialized along with the command line string.
 2. The *sshell$* prompt is output to the terminal and the user inputs a command and presses enter.
 3. The command is then fed through our parser. 
 4. The parsed commands, now individual processes are compared against other commands to determine the desired process to be run.
 5. If none of the compared commands are matched, it is sent into the pipe function where it will be executed using *execvp*.
 6. Once the commands are run, the while loop will continue until the user exits.

## Specific Implementations

**Data Structures:** In this program, we used two specific data structures for the critical implementations of the shell: *Process* and *Task*.
**Process:** This is implemented as a struct to organize the information pertaining to each individual command. It contains three variable: 

 - *args[ ]:* This array of strings stores the arguments and the NULL pointer that make up a single command that *execvp* can execute (ex: {"echo", "hello", (char*)NULL} ).
 - 	*opp:* This character holds the operator (either '|' or '>') that proceeds an individual command.
 - 	*opp_err:* This character holds either nothing or the '&' symbol to denote if the operation will be redirected to standard error.
 
 **Task:** This struct holds the data pertaining to all commands gathered from the command line input. There is only ever one Task during the functions entire life cycle. It contains three variables:
 - *processes[ ]*: This array holds a pointer to each Process. 
 - *num_pros:* This integer represents the number of Process currently in processes[ ].
 - *num_pipes:* This integer represents the number of piped commands in processes[ ].
---
**Command Parsing:** Parsing is done using the *PARSE_CMD( )* function. This function takes in the command line and a Task as arguments. It begins by creating a blank Process, a temporary string, and integers to keep count of the process, argument, and character. The process counter is an index for the Task just as the argument counter is an index for the Process arguments. The character counter is to adjust for the main index of the for loop. The for loop moves character-by-character up to max 512 characters allowed in the command line. Each iteration the parser looks for either spaces, operator characters, or a null or newline character. If none of those are found it will add that character to the temporary string. If it encounters a space it will look at the previous index to see if it also contained a space. If it did, it will skip that space and move on to the next index. If no space was at the previous index, it assumes that the string is complete and adds it to Process. The temporary string will be added to Process if it encounters a newline or null character as well. When the function encounters a newline or null character it will add that Process to Task and the the num_pros will be incremented and the loop breaks. When an operator character is found, it will store the operator in the Process opp variable. If that operator is followed by an ampersand, it will add that to the opp_err variable. Then it will ignore any extra spaces and add that string to Process, then add that Process to Task. This cycle continues until the function breaks or it cycled through all 512 character of the command line.
This function seems to work correctly, however we believe there to be an issue with adding the (char*)NULL to the end of the Process arguments.

**Piping:** The piping is done by a function called *PIPE_CMD( )* which takes the Task as its argument. It begins by creating a pid value, an array for file descriptors, and two integer values to store the previous fd_in and current fd_in. A for loop from zero to the num_pros of the Task is started cycling through each Process in Task using the for loop index. The pipe is created with fd  array and the process is forked. The parent and child both have three cases: the Process is the first process, the process is in the middle, and the process is the last process. Depending on which process is currently being looked at, the function will connect the pipes as necessary and execute the commands after set up. 
This function works correctly when given a static manual input, so we believe this function is implemented correctly. However based on the issues with described with the parsing, it doesn't pipe correctly. 

**Output Redirect:** This command was never fully implemented due to issues with piping and time constraints.

**exit:** This was implemented using a simple function that took in the 'exit' command from *Task*, returns the default return string ("+ completed '*command*' [*rtn_val*]"), and  breaks from the while loop, ending the program.

**pwd:** This function utilized the *getcwd( )* function from the *unistd.h* header and returned the resulting string to the terminal.

**cd:** When the user inputed ".." as the path, the cd function called *getcwd( )* and returns the current directory into a string. The string is then run though a for loop placing the updated index of the most recent '/' character. When the loop ends, the current directory is then run through *strcpy( )* keeping only the character up until the index of the last '/' character. Thus removing the most recent directory. This new directory is placed used with the *chdir( )* function to change the users current directory.

---
**Directory Stack:**
For the directory stack, we created a structure of a StackNode utilizing the *PushStack(), PopStack(), Display(), isEmpty()* functions. 

**StackNode:** Is a structure which chained together creates a linked-list that contains two variables:

 - *nodeVal:* This is a character pointer that points to the string of the directory.
 - *nextNode:* This is a StackNode pointer that points to the next node in the list. 

**pushd**: In *main()*, when pushd is entered by the user, it calls the *PushStack()* function, which takes the current directory using *getcwd( )*, creates a new instance of StackNode, and adds it to the stack.
**popd** : In *main()*, when popd, is called, it runs the *PopStack()* function, which returns the latest directory from the stack and uses *chdir( )* to make that directory the current directory. 
**dirs**: In *main(),* when dirs is called, it runs the *display()* function, which list the current directory followed by all of the directories stored in the stack.
**isEmpty()** is a boolean function that returns true if the stack is empty and gets used to print "Error: directory stack empty" when a stack gets first created or when the stacked gets all of its elements popped and the user wants to continue popping.  

**Standard error redirection:** This command was never fully implemented due to issues with piping and time constraints.
