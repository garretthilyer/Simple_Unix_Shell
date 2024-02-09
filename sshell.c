#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <fcntl.h>

#define CMDLINE_MAX 512
#define CMDARGS_MAX 17

void runShell();  //  so any function can call the shell if needed

struct commandArgs {
        char* commandArray[CMDARGS_MAX];  //  set to 17 because = 16 Args max + NULL char in final index 
        char* file;
        struct commandArgs* Next;  //  points to next 
        int fileType;  //  STDIN = 0 STDOUT = 1
};

void errorCatcher(int caseNum){
        // take in different error codes for inputs
        // Contain a switch for all the different error situations (match error code to solution)
        switch(caseNum){
                case 1:
                        // output redirection error
                        fprintf(stderr, "Error: misalocated output redirection\n");
                        break;
                case 2:
                        // input redirection error
                        fprintf(stderr, "Error: misalocated input redirection\n");
                        break;
                case 3:
                        // enter valid input error
                        fprintf(stderr, "Error: enter valid input\n");
                        break;
                case 4:
                        // missing command func
                        // look at paring func for
                        fprintf(stderr, "Error: missing command\n");
                        break;
                case 5:
                        // no output file error
                        fprintf(stderr, "Error: no output file\n");
                        break;
                case 6:
                        // no input file error
                        fprintf(stderr, "Error: no input file\n");
                        break;
                case 7:
                        // too many argumements
                        fprintf(stderr, "Error: too many process arguements\n");
                        break;
                case 8:
                        // cannot open input file error
                        fprintf(stderr, "Error: cannot open input file\n");
                        break;
                case 9:
                        // command not found error
                        fprintf(stderr, "Error: command not found\n");
                        break;
                case 10:
                        // cannot cd into directory error
                        fprintf(stderr, "Error: cannot cd into directory\n");
                        break;
                case 11:
                        // no such directory error
                        fprintf(stderr, "Error: no such directory\n");
                        break;
                case 12:
                        // directory stack empty error
                        fprintf(stderr, "Error: directory stack empty\n");
                        break;        
        }
}

struct dirStack{
    char* currDir;  //  latest path added to directory stack
    struct dirStack *prev;  //  pointer to last directory path 
};

int* threeCommands(struct commandArgs command1, struct commandArgs command2, struct commandArgs command3){
        static int returnValues[3]; 
        /* Catches all possible input/out redirection errors */
        if( command1.fileType == 1 || command3.fileType == 0 || command2.fileType != -1){
                // ERROR CATCHER
        }

        int fd1[2];  //  pipe one  (command1 <--> command2)
        int fd2[2];  //  pipe two  (command2 <--> command3)
        
        pipe(fd1); 
        pipe(fd2);

        pid_t firstFork = fork();  //  first fork 
        if(firstFork == 0){  //  first child process (command 1)
                /*close all unncessary pipes */
                close(fd2[0]);  
                close(fd2[1]);
                close(fd1[0]);

                /* if input for first command needs to be redirected */
                if(command1.fileType == 0){
                        int fdRedirect;
                        fdRedirect = open(command1.file, O_RDONLY | O_CREAT, 0644);
                        dup2(fdRedirect, STDIN_FILENO);
                        close(fdRedirect);
                }
                /* replace outputs first command with pipe*/
                dup2(fd1[1], STDOUT_FILENO);

                close(fd1[1]);
                execvp(*(command1.commandArray), command1.commandArray);

        } else {
                pid_t secondFork = fork();
                if (secondFork == 0) {  //  second child process (command 2)
                        close(fd1[1]);
                        close(fd2[0]);

                        dup2(fd1[0], STDIN_FILENO);
                        dup2(fd2[1], STDOUT_FILENO);

                        close(fd1[0]);
                        close(fd2[1]);

                        execvp(*(command2.commandArray), command2.commandArray);
                } else {
                        pid_t thirdFork = fork();
                        if (thirdFork == 0) {  //  third child process (command 3)
                                close(fd1[0]);
                                close(fd1[1]);
                                close(fd2[1]);

                                /*if command 3 needs its output redirected*/
                                if(command3.fileType == 1){
                                        int fdRedirect;
                                        fdRedirect = open(command3.file, O_WRONLY | O_CREAT, 0644);
                                        dup2(fdRedirect, STDOUT_FILENO);
                                        close(fdRedirect);
                                }

                                dup2(fd2[0], STDIN_FILENO);
                                close(fd2[0]);

                                execvp(*(command3.commandArray), command3.commandArray);
                        } else {  //  parent proccess 
                                close(fd1[0]);  //  close all pipes in parent process
                                close(fd1[1]);
                                close(fd2[1]);
                                close(fd2[0]);

                                int status1;  //  initialize all status returns 
                                int status2;
                                int status3;

                                waitpid(firstFork, &status1, 0);
                                waitpid(secondFork, &status2, 0);
                                waitpid(thirdFork, &status3, 0);
                                
                                returnValues[0] = status1;  //  assign return status to each element in array
                                returnValues[1] = status2;
                                returnValues[2] = status3;

                        }
                }
        }

       return returnValues; 
}

/* two commands follows nearly identical idea to three commands except one less fork & pipe*/
int* twoCommands(struct commandArgs command1, struct commandArgs command2) {
        static int returnValues[2];
        if(command1.fileType == 1 || command2.fileType == 0){
                //ERROR CATCHER
        }

        int fd[2];
        pipe(fd);
        pid_t firstFork = fork();

        if (firstFork == 0 ) {   
                close(fd[0]);
                if(command1.fileType == 0){
                        int fdRedirect;
                        fdRedirect = open(command1.file, O_RDONLY | O_CREAT, 0644);
                        dup2(fdRedirect, STDIN_FILENO);
                        close(fdRedirect);
                }

                dup2(fd[1], STDOUT_FILENO); 

                close(fd[1]); 
                execvp(*(command1.commandArray), command1.commandArray);
                
        } else {
                pid_t secondFork = fork();
                if(secondFork == 0){
                        close(fd[1]);
                        if(command2.fileType == 1){
                                int fdRedirect;
                                fdRedirect = open(command2.file, O_WRONLY | O_CREAT, 0644);
                                dup2(fdRedirect, STDOUT_FILENO);
                                close(fdRedirect);
                        }

                        dup2(fd[0], STDIN_FILENO);
                        
                        close(fd[0]);
                        execvp(*(command2.commandArray), command2.commandArray);
                }else{
                        close(fd[0]);
                        close(fd[1]);
                        int status1;
                        int status2; 

                        waitpid(firstFork, &status1, 0);
                        waitpid(secondFork, &status2, 0); 
                        returnValues[0] = status1;
                        returnValues[1] = status2;
                       
                }
        }
        
        return returnValues;
}

int singleCommand(struct commandArgs onlyCommand){
        pid_t pid;
        pid = fork();
        if(pid != 0) {  //  parent process 
                int status;  
                wait(&status);   //  wait for child to exit and get return value
                return status;
        } else {  //  child process

                if(onlyCommand.file != NULL){
                        int fd;
                        if (onlyCommand.fileType == 0) {
                                fd = open(onlyCommand.file, O_RDONLY | O_CREAT, 0644);
                                dup2(fd, STDIN_FILENO);
                                close(fd);

                        } else if(onlyCommand.fileType == 1) {
                                fd = open(onlyCommand.file, O_WRONLY | O_CREAT, 0644);
                                dup2(fd, STDOUT_FILENO);
                                close(fd);
                        }
                }

                //execvp((const char*)cmd, (char *const)cmd);
                execvp(*(onlyCommand.commandArray), onlyCommand.commandArray);
                int execReturn = execvp(*(onlyCommand.commandArray), onlyCommand.commandArray);
                if(execReturn == -1) {
                        return -1;
                }
                exit(0); // run new program in child process
        }
}

void pwdCommand() {
        char buffer[CMDLINE_MAX];
        if(getcwd(buffer, sizeof(buffer)) != NULL){
                printf("%s\n", buffer);  //  possible I am printing to wrong std location (out instead of err) 
        } 
}
 
int parseCMD(char* cmd, struct commandArgs* commands){
        int counter = 0;
        int redirectDetector = 0;
        int pipelineDetector = 0;
        int cmdLineInput = 0;
        int prePipelineInput = 0;
        char lastSeen;
        char** arrayPtr = commands->commandArray;  //  assign a double ptr to the command array 
        while(*cmd != '\0'){
                /*replace all white spaces with NULL character to remove and seperate*/
                while (*cmd == ' ' || *cmd == '\t' || *cmd == '\n') {
                        *cmd++ = '\0';
                }
                /*I/O redirection check*/
                if (*cmd == '>' || *cmd == '<') {
                        redirectDetector = 1;
                        lastSeen = *cmd;

                        if(*cmd == '<'){
                                commands->fileType = 0; 
                        }else{
                                commands->fileType = 1;
                        }
                        *cmd++ = '\0';
                        while (*cmd == ' ' || *cmd == '\t' || *cmd == '\n') {
                                *cmd++ = '\0';
                        }
                        commands->file = cmd;
                /*Pipeline checking and recursion call*/
                } else if(*cmd == '|') {
                        pipelineDetector ++;
                        if(cmdLineInput != 0){
                                prePipelineInput = cmdLineInput;
                        }else{
                                prePipelineInput++;  
                        }
                        cmdLineInput = 0;
                        *cmd++ = '\0';
                        struct commandArgs nextCommand;
                        nextCommand.file = NULL;
                        nextCommand.Next = NULL;
                        nextCommand.fileType = -1;
                        commands->Next = &nextCommand;
                        parseCMD(cmd, commands->Next);
                        
                } else if(*cmd != '\0') {
                        counter ++;
                        if(counter > (CMDARGS_MAX - 1)){
                                return 7;
                        } 
                        *arrayPtr++ = cmd;
                }
                /* check for anything that would break up the argument pasrsing*/
                while (*cmd != ' ' && *cmd != '\t' && *cmd != '\n' && *cmd != '\0' && *cmd != '>' && *cmd != '<' && *cmd != '|') {
                        lastSeen = *cmd;
                        cmd++;
                        cmdLineInput++;
                        if((redirectDetector == 1 )&&(cmdLineInput == 1)){
                                return 4;
                        }
                        if((pipelineDetector != 0)&&(cmdLineInput == 1)&&(lastSeen == '|')){
                                return 4;
                        }
                }
        }
        *arrayPtr++ = '\0';  //  set last char* in command array to NULL for execvp call
        if((lastSeen == '|') && (prePipelineInput != 0)){
                return 4;
        }
        if((lastSeen == '>') && (cmdLineInput != 0)){
                // no output file error
                return 5;
        }
        if((lastSeen == '<') && (cmdLineInput != 0)){
                // enter input file error
                return 6;
        }
        if(((lastSeen == '>') || (lastSeen == '<')) && (cmdLineInput == 0)){
                // enter valid input error (only > or < was entered)
                return 3;
        }
        return 0;
}

void runCommand(char* cmd, struct dirStack* directory){

        char* rawLine = malloc(strlen(cmd)+1);
        strcpy(rawLine, cmd);  //  copy unparsed commandline so it can be printed to 

        struct commandArgs firstCommand;
        firstCommand.file = NULL;
        firstCommand.Next = NULL;
        firstCommand.fileType = -1;
        
        int parseReturn = parseCMD(rawLine, &firstCommand);

        int commandOneFileType = -2;
        int commandTwoFileType = -2;
        int commandThreeFileType = -2;
        commandOneFileType = firstCommand.fileType;
        if(firstCommand.Next != NULL){
                commandTwoFileType = firstCommand.Next->fileType;
                if(commandOneFileType == 1){
                        // misallocated output redirection error
                        errorCatcher(1);
                        runShell(*directory);
                }
                if(commandTwoFileType == 0){
                        // missallocated input redirection error
                        errorCatcher(2);
                        runShell(*directory);
                }
                if(firstCommand.Next->Next != NULL){
                        commandThreeFileType = firstCommand.Next->Next->fileType;
                        // if command 1 has output redirect, command 2 has either, or command 3 has input redirect
                        if(commandOneFileType == 1 || commandTwoFileType == 1){
                                // misallocated output redirection error
                                errorCatcher(1);
                                runShell(*directory);
                        }
                        if(commandThreeFileType == 0 || commandTwoFileType == 0){
                                // missallocated input redirection error
                                errorCatcher(2);
                                runShell(*directory);
                        }
                }
        }

        if(parseReturn != 0){
                errorCatcher(parseReturn);
                runShell(*directory);
        }
        int builtinReturn;
        
        /* PWD BUILT IN COMMAND */
        if(!strcmp(firstCommand.commandArray[0], "pwd")) {

                pwdCommand();
                fprintf(stderr, "+ completed '%s' [0]\n", cmd);
                

        /* CD BUILT IN COMMAND */
        } else if (!strcmp(firstCommand.commandArray[0], "cd")) {

                builtinReturn = chdir(firstCommand.commandArray[1]);
                if(builtinReturn == -1){
                        errorCatcher(10);
                        fprintf(stderr, "+ completed '%s' [1]\n", cmd);
                        runShell(*directory);
                }else{
                        fprintf(stderr, "+ completed '%s' [0]\n", cmd);
                }
                
                
        /* dirs BUILT IN COMMAND */
        } else if (!strcmp(firstCommand.commandArray[0], "dirs")) {

                pwdCommand();
                struct dirStack trace = *directory;
                while(trace.currDir != NULL){
                        printf("%s\n", trace.currDir);
                        trace = *trace.prev;
                }
                fprintf(stderr, "+ completed '%s' [0]\n", cmd);

        /* pushd BUILT IN COMMAND */
        } else if (!strcmp(firstCommand.commandArray[0], "pushd")) {

                struct dirStack* lastDirectory = malloc(sizeof(struct dirStack*));
                lastDirectory->currDir = directory->currDir;
                lastDirectory->prev = directory->prev;
                char buffer[CMDLINE_MAX];

                if(getcwd(buffer, sizeof(buffer)) != NULL){
                        directory->currDir = buffer; 
                        directory->prev = lastDirectory;
                } 

                builtinReturn = chdir(firstCommand.commandArray[1]);
                // if cdir returns error free mallocd mem, call error catcher and runshell on the null &directory
                if(builtinReturn == -1){
                        errorCatcher(11);
                        fprintf(stderr, "+ completed '%s' [1]\n", cmd);
                        return;
                }else{
                        fprintf(stderr, "+ completed '%s' [0]\n", cmd);
                }
                
        /* popd BUILT IN COMMAND */
        } else if (!strcmp(firstCommand.commandArray[0], "popd")) {

                if (directory->currDir == NULL) {
                        fprintf(stderr, "Error: directory stack empty\n+ completed 'popd' [1]\n");
                } else {
                        chdir(directory->currDir);
                        directory->currDir = NULL;
                        directory = directory->prev;
                        fprintf(stderr, "+ completed '%s' [0]\n", cmd);
                }

        /* exit BUILT IN COMMAND */
        } else if (!strcmp(firstCommand.commandArray[0], "exit")) {
                
                fprintf(stderr, "Bye...\n");
                fprintf(stderr, "+ completed '%s' [0]\n", cmd);
                exit(0);

        }else{  //  non built command calls 
                if(firstCommand.Next == NULL) {
                        /* logical branch for one command*/
                        int oneReturn = singleCommand(firstCommand);
                        if(oneReturn == -1){
                                struct dirStack recurDirectoryStack = {NULL, NULL};
                                errorCatcher(9);
                                fprintf(stderr, "+ completed '%s' [1]\n", cmd);
                                runShell(recurDirectoryStack);
                        }
                        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, oneReturn);

                }else if(firstCommand.Next->Next == NULL) {
                        /*logical branch for two commands pipelined*/
                        struct commandArgs command2 = *firstCommand.Next;
                        int* twoReturn = twoCommands(firstCommand, command2);
                        fprintf(stderr, "+ completed '%s' [%d][%d]\n", cmd, twoReturn[0], twoReturn[1]);
                        
                }else{
                        /*logical branch for three commands pipelined*/
                        struct commandArgs command2 = *firstCommand.Next;
                        struct commandArgs command3 = *firstCommand.Next->Next;
                        int* threeReturn = threeCommands(firstCommand, command2, command3);
                        fprintf(stderr, "+ completed '%s' [%d][%d][%d]\n", cmd, threeReturn[0], threeReturn[1], threeReturn[2]);
                        
                }
                
        }
        free(rawLine);
}

void runShell(struct dirStack directory){
        char cmd[CMDLINE_MAX];

        while(1){
                char *nl;

                /* Print prompt */
                printf("sshell@ucd$ ");
                fflush(stdout);
                fflush(stderr);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';
                if(cmd[0] == '\0'){
                        return;
                }

                /* Regular command */
                runCommand(cmd, &directory);
        }
        
}

int main(void)
{       
        struct dirStack directoryStack = {NULL, NULL};
        runShell(directoryStack);
        
        return EXIT_SUCCESS;
}
