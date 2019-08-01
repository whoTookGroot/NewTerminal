/*
==================================================================================
 * Author: Erik Grootendorst
 * Class: CS570
 * Prof: Carroll
 * Date: 10/4/18
 * Description: Simple command line interpreter intended for ssh or bash.
 =================================================================================
 */

#include "p2.h"


int main(void){
    //system("open -a Terminal.app");

    const char cd_built_in[] = "cd";
    const char exit_built_in[] = "exit";
    const char PROMPT[] = ":570:";

    //holds args, buffer holds entire line.
    char *word_args[MAXIMUM];
    char buffer[MAX_STORAGE];
    int exitCode;

    setpgrp();
    signal(SIGTERM,sighandler);
    
    //begin CLI
    for(;;){

        exitCode = 0;
        resetAll(word_args, buffer);
        printf("%s ", PROMPT);

            //reissue prompt if no input was submitted
            if((exitCode = parse(word_args,buffer)) == 0)
                continue;

            else if(exitCode == EOF)
                exitHandle();
            
            else if(exitCode < -1){
                errorHandle(exitCode);
                continue;
            }

        //check for cd built-in
        if(strcmp(word_args[0],cd_built_in) == 0){
            if((exitCode = cdHandle(word_args)) < 0)
                errorHandle(exitCode);

            continue;
        }
            
            

        //terminate program
        if((strcmp(word_args[0],exit_built_in) == 0) && exitCode == EXIT)
            exitHandle();
        
        else
            exec_command(word_args);
         

        //resetAll(word_args);
        
    }

    return (0);
}

//FIXME Construct command strings and set global flags for fork(), error message for syntax
int parse(char** commands, char* buffer){
    
    int charCount = 0;
    int wordCount = 0;
    
    //global getword() variables
    EOL_REACHED = 0;
    backSlash = 0;

    //parse command line and poppulate string array
    do{

        charCount = getword(buffer);
        clearStdinFlag = charCount;

        if(charCount == -255){
            wordCount = EOF;
            break;
        }

        else if(*buffer == '\0' && !backSlash)
            break;

        //check for invalid & syntax
        else if(*buffer == '&' && wordCount == 0 && !backSlash){
            wordCount = INVALID_AMP_SYNTAX;
            break;
        }

        //check for multiple pipes::: add pipe flag, seperate process
        else if(*buffer == '|'){

            if(pipe_set > 0 || wordCount == 0){
                return PIPE_ERROR;
            }

            //flag syntax such as '< somefile'
            if(redirect_in && argCount == 0){
                return INVALID_REDIRECT_IN;
            }

            //check for & just before pipe (end of first process)
            pipe_set = argCount;
            *commands++ = NULL;
            //increment to set current word_[argCount] == '\0'
            argCount++;
        }

        else if(strcmp(buffer,"<") == 0){

            if(redirect_in){
                return AMBIGUOUS_REDIRECT_IO;
            }

            redirect_in = TRUE;
        }

        else if(redirect_in && *inputFile == '\0')
            strcpy(inputFile,buffer);

         //dont need to strcmp() as '>' is a unique [single] symbol
        else if(*buffer == '>'){

            if(redirect_out /*|| wordCount == 0*/){
                return AMBIGUOUS_REDIRECT_IO;
            }

            redirect_out = TRUE;
        }

        //set outFile
        else if (redirect_out && *outFile == '\0')
            strcpy(outFile,buffer);

        else{

            //check for a environment variable ($HOME)
            if(charCount < 0)
                charCount = -1*charCount;

            *commands++ = buffer;
            *commands = NULL;
            buffer += charCount + 1;
            argCount++;
        }
        wordCount++;
    } 
    while(EOL_REACHED != 1 && wordCount < 100);
    
    //check for & on end of line and remove from word_args
    if(argCount > 1 && *(buffer-2) == '&' && !backSlash){
        background_proc = TRUE;
        *(commands-1) = NULL;
        argCount--;
    }
    
    //check for invalid pipe syntax:: 'process1 | ', commands-(pipe_set+2) to read command after '|'
    if(pipe_set > 0 && *(commands - (pipe_set+2)) == NULL)
        return PIPE_ERROR;

    //flag syntax such as '< somefile'
    if(redirect_in && argCount == 0)
        return INVALID_REDIRECT_IN;
    
    //redirect with no command
    if(redirect_out && argCount == 0)
        return INVALID_REDIRECT_IN;

    //redirect with no outfile
    if(redirect_out && *outFile == '\0')
        return MISSING_REDIRECT;
    
    return wordCount;
}


//built-in for 'cd'
int cdHandle(char** commands){
    int exitCode = 0;
    
    //check to see for too many args, if pipe_set > 2 it means cd has > 1 arg
    if((pipe_set == 0 && argCount > 2) || (pipe_set > 2))
        return TOO_MANY_CD_ARGS;

    //check for specified path
    if(argCount > 1)
        if(*commands[1] != '\0')
            return chdir(commands[1]);

    return chdir(getenv("HOME"));
}

//built-in for 'exit'
void exitHandle(){
    //send kill to processgroups, exit gracefully
    killpg(getpgrp(), SIGTERM);
    printf("p2 terminated.\n");
    exit(0);
}

void errorHandle(int error){
    
    switch(error){
        case PIPE_ERROR:
            perror("ERROR:Invalid pipe syntax.");
            break;
        case CD_FAIL:
            perror("ERROR: cd failed, check syntax.");
            break;
        case AMBIGUOUS_REDIRECT_IO:
            perror("ERROR: Ambiguous redirect.");
            break;
        case PERMISSION_DENIED:
            perror("Permission Denied.");
            break;
        case NO_DIRECTORY_FOUND:
            perror("No Directory Found.");
            break;
        case INVALID_AMP_SYNTAX:
            perror("Invalid ampersand syntax.");
            break;
        case FORK_FAILED:
            perror("Failed to fork.");
            break;
        case FILE_ALREADY_EXISTS:
            perror("File already exists, cannot overwrite.");
            break;
        case COULD_NOT_CREATE_FILE:
            perror("Output file could not be created.");
            break;
        case REDIRECT_FAILED:
            perror("Redirect failed.");
            break;
        case EXEC_COMMAND_FAILED: 
            perror("Command failed to execute.");
            break;
        case INPUT_FILE_ERROR: 
            perror("Error opening input file.");
            break;
        case INPUT_REDIRECT_ERROR: 
            perror("Could not redirect input.");
            break;
        case FILE_NOT_FOUND:
            perror("File not found.");
            break;
        case FAILED_TO_PIPE: 
            perror("Failed to pipe.");
            break;
        case PIPE_REDIRECT_FAILED: 
            perror("Failed to redirect pipe.");
            break;
        case PIPE_LEFTHAND_FAILED: 
            perror("Left process of pipe failed to redirect.");
            break;
        case EXEC_CHILD_COMMAND_FAILED: 
            perror("Child process failed to execute.");
            break;
        case INVALID_REDIRECT_IN: 
            perror("Invalid syntax using redirect-in.");
            break;
        case INVALID_REDIRECT_OUT: 
            perror("Invalid syntax using redirect-out.");
            break;
        case TOO_MANY_CD_ARGS: 
            perror("Too many cd args.");
            break;
        case MISSING_REDIRECT: 
            perror("Missing name for redirect.");
            break;

    }
    fflush(stderr);
}

void exec_command(char** commands){

    pid_t cpid;
    pid_t child_status;
    int exitCode = SUCCESS;
    fflush(stdout);
    fflush(stderr);

    //fork child process
    if((cpid = fork()) == 0)
        exec_child_command(commands);

    //have parent wait if & isnt set
    else{

        if(!background_proc){

            do{
                child_status = wait(NULL);
            }
            while(child_status != cpid);
        }

        else{
            printf("%s [%d]\n",commands[0],cpid);
        }
    }
    
}

void exec_child_command(char **commands){
    
    //post fork, check for redirection
    if(redirect() < 0){
        errorHandle(REDIRECT_FAILED);
        exit(REDIRECT_FAILED);
    }

    //check for pipe
    else if(pipe_set > 0)
        exec_pipe(commands);
    
    else{
        if(execvp(commands[0],commands) < 0){
            errorHandle(EXEC_CHILD_COMMAND_FAILED);
            exit(EXEC_CHILD_COMMAND_FAILED);   
        }
    }
    exit(0);
}

//fix redirect in
int redirect(){

    int exitCode = SUCCESS;
    //int fildescriptors
    int fd;

    //check for outFile
    if(*outFile != '\0'){

        //make sure it doesnt exist already
        if(access(outFile,F_OK) == 0)
            return FILE_ALREADY_EXISTS;

        else{
            
            //create file for writing, set read and right permission bits
            if ((fd = open(outFile, O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR)) < 0){
                errorHandle(COULD_NOT_CREATE_FILE);
                exit(COULD_NOT_CREATE_FILE);
            }

            //redirect stdout
            if(dup2(fd,1) < 0){
                errorHandle(REDIRECT_FAILED);
                exit(REDIRECT_FAILED);
            }
        }
        
        close(fd);
    }

    if(*inputFile != '\0'){

        //check if file exists
        if(access(inputFile, F_OK) != 0)
            return  FILE_NOT_FOUND;
        
        //check if file can be read
        if(access(inputFile, R_OK) != 0){
            errorHandle(PERMISSION_DENIED);
            return PERMISSION_DENIED;
        }

        //check if open with read permissions succeeded
        if((fd = open(inputFile, O_RDONLY)) < 0){
            errorHandle(INPUT_FILE_ERROR);
            exit(INPUT_FILE_ERROR);
        }
       
        //redirect stdin
        if(dup2(fd,0) < 0){
            errorHandle(INPUT_REDIRECT_ERROR);
            exit(INPUT_REDIRECT_ERROR);
        }

        close(fd);
    }

    //handle case when no input file is given and background process is set.
    //avoid processes competeting for keyboard input.
    if(*inputFile != '\0' && background_proc){
        
        if((fd = open("/dev/null", O_RDONLY)) < 0){
            errorHandle(INPUT_FILE_ERROR);
            exit(INPUT_FILE_ERROR);
        }
        
        if(dup2(fd,0) < 0){
            errorHandle(INPUT_REDIRECT_ERROR);
            exit(INPUT_REDIRECT_ERROR);
        }

        close(fd);
    }


    return exitCode;
}


void exec_pipe(char** commands){
    //setup filedescriptors
    int fd[2];
    
    pipe( fd );
    if( fork() == 0 ) {
        
        //redirect stdout
        if(dup2( fd[1], 1) < 0){
            errorHandle(PIPE_LEFTHAND_FAILED);
            exit(PIPE_LEFTHAND_FAILED);
        }
        close( fd[0] ); 
        close( fd[1] );

        if(execvp(commands[0],commands) < 0){
            errorHandle(PIPE_LEFTHAND_EXECUTE);
            exit(PIPE_LEFTHAND_EXECUTE);
        } 
        //exit(2)
        exit(0);
    } 
    else {
        //redirect stdin
        if(dup2( fd[0], 0) < 0){
            errorHandle(PIPE_RIGHTHAND_FAILED);
            exit(PIPE_RIGHTHAND_FAILED);
        }
        close( fd[0] ); 
        close( fd[1] );
        //execute pipe commands located with the pipe_set + 1 offest
        if(execvp(commands[pipe_set+1],commands + (pipe_set + 1)) < 0){
            errorHandle(PIPE_RIGHTHAND_EXECUTE);
            exit(PIPE_RIGHTHAND_EXECUTE);
        } 
        
        exit(0);
    }
    
}

void resetAll(char** commands, char* buffer){
    pipe_set = FALSE;
    redirect_in = FALSE;
    redirect_out = FALSE;
    ambiguous_redirect = FALSE;
    background_proc = FALSE;
    argCount = 0;
    *commands = NULL;

    //clear stdin since fflush(stdin) doesnt work
    //and fpurge(stdin) doesnt work on edoras
    //and fseek(stdin,1,SEEK_SET) doesnt work on edoras
    if( clearStdinFlag > 0)
        while(getword(buffer) != 0)
            continue;
    
    *buffer = '\0';
    *outFile = '\0';
    *inputFile = '\0';

    fflush(stderr);
    fflush(stdout);
    fflush(stdin);

}

void sighandler(int signumber){
    errorHandle(signumber);
}
