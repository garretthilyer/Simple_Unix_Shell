## Introduction
------
The overall goal of this programming project was to strengthen the understanding of system calls such as fork(), exec(), wait() and other basic UNIX system calls. This was achieved by building a very low-functioning shell called *sshell*. This shell could receive up to 3 command line arguments from the user that were pipelined togther and execute the correct output. 

## Implementation 
------
The implementation follows a straightforward strategy of parsing and encapsulating a command line into a data structure that is then used to execute specific instructions either by using a systen call or a builtin command in the program.

### Data Structures
There were two main data structures used in this implmentation. Both of these structures were of type *struct*, but served entirely different purposes. One was used to stored a parsed commandline (*commandArgs*), while the other structure was used to create a linked list that held the directory stack (*dirStack*). 

#### commandArgs
As stated above, the sole purpose of *commandArgs* is to encapsulate different pieces of data that are needed in order to properly execute a command. The first and probably most important of which is the char* array *commandArray*. This array is used to properly hold command strings in an array that can properly be executed by execvp if and when needed. The size of *commandArray* is held to a constant 17 (16 arguments max + NULL character). *commandArray* is the only data that is required to be filled for a proper command to run. The next piece of data is char* *file* which holds a possible file input/output redirection if needed. *file* is not always required and is initialized to NULL. The next piece of data is int *filetype*, which is set to 0 if the file is used for input redirection, 1 for output, but initialized to -1 when the command that is ran doesn't use redirection. The fourth and final data structure inside *commandArgs* is a pointer to another *commandArgs* structure called *Next*. This essentially creates a linked list between commands. *Next* is initialized to NULL and only ever points to another *commandArgs* struct when dealing with pipeling commands 

#### dirStack
The second data structure used is called *dirstack*. This struct is used to hold onto a char* called *currDir* which holds the string of the latest path added to the directory stack. The other piece of data in the *dirstack* struct is a pointer to another *dirStack* struct called *prev*. This also creates a linked list and makes it possible to navigate through the stack to access and manipulate data when calling functions such as *dirs*, *pushd* & *popd*. 

### Parsing
The parsing function used in this project takes in a raw command string and breaks it down into specific elements that are split up either by white spaces, file redirection, or pipeline indicators. Upon a white space, the parsing function *parseCMD* will set the specified whiteline, newline, or tab character to '\0' in order to completely remove it from the string. If we traverse over the file indicator, the function will detect if it wants input or output and set the *fileType* to the specified number and set *file* to the remaining string. 

```C
if (*cmd == '>' || *cmd == '<') {
  if(*cmd == '<'){
    commands->fileType = 0; 
  } else {
    commands->fileType = 1;
  }
}
```
The parser also checks for the pipeline symbol which would then initialize a new *commandArgs* struct that is pointed to by the current *commandArgs* struct. This struct is then passed into a recursion call of *parseCMD*. By this logical, our praser could initialize a structure that can handle as many commands in a pipeline as we want (although the data structure wouldn't run correctly on code later on. 

### Executing Commands
In the *runCommand* function the newly parsed and implemented data structure goes through a logical if/else statement to find the correct command to run. If the first char* of the *commandArray* function matches with any of the builtin functions (i.e *cd, pwd, exit, dirs, pushd, popd*), the function will run that branch in order to execute the specified instruction. 

In most cases however, the if/else will reach the bottom where it then determines if the function is a single command, two commands, or three commands. This is done by checking if the *Next* struct pointer is set to NULL. Depending on how many nodes are in the linked list will determine which of the three functions to run.

#### One Command
The *singleCommand* function is very straight forward. It forks the process and checks for file redirection in the child proccess and runs execvp in order to execute the command. The parent function then returns the exit status of the child for *stderr* to print out. 

#### Two Commands
The *twoCommands* function requires some more thought. You now need to implement a pipe and fork twice. The first fork allows for the first command to run. We also connect the output of this command to the pipe. The first command also needs to check for file input redirection. Inside the parent process, you fork for a second time and create a second child process which will run the second command. Inside the second child process you need to check for file output redirection and connect the input of this command to the pipe in order to read the first commands output. Both child processes also run execvp to execute their own commands. The parent process waits for the exit status of both commands and returns it in an array. 

#### Three Commands
The third and final of the three possible command executions (*threeCommands*) follows a near identical structure to *twoCommands*, except their is now two pipelines and inside the parent process the program will fork for a third time that allows for a third child process to execute command three. The second command connects it's input to the first pipe and its output to the second pipeline (*fd1[0] & fd2[1]*). Ovbiously, commands one and three need to check for input and output redirection respectfully. 

### Error Catching 
The basic premise of the error catching function is that conditional statements were placed in places of key importance where exceptions were likely to arise. This includes our parsing (*ParseCMD*) function and each function that handled 1, 2,and 3 commands as input respectively. In ParseCMD we used a variety of different int and char variables to detect when certain inputs were seen and depending on the combination of said inputs, whether an error was raised or not.

## Conclusion
------
This project really strengthened our knowledge of system calls ands pipelining commands into the shell. The most difficult task was implementing the *threeCommands* function in order to deal with two pipelines and three instructions. We didn't realize you needed to close the pipelines inside the parent process as well which wasted a lot of precious time. Another tricky part of this assignment was surprisingly, implementing the directory stack. The directory stack itself was not that challenging by recoding certain functions to pass the directory stack through. At one point we debated just making the structure a *global struct*, but that is just lazy coding. The project as a whole took around 25+ hours over the span of a week. 
