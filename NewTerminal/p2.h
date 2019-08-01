/*
==================================================================================
 * Author: Erik Grootendorst
 * Class: CS570
 * Prof: Carroll
 * Date: 10/4/18
 * Description: Simple command line interpreter. Repeatedly prompts for input until EOF is reached.
 * Reads from stdin. Reads in a maximum of 100 words, 254 characters per word. Parses command line with
 * getword() and constructs an array of words checking for syntactic integrity. It handles simple meta characters such
 * as '/','&','<','>','|'. It then sets about handling the command and its arguments. After parsing the array it 
 * prepares to execute the commands. It sets up and necessary redirection, checks for pipes, and executes. Error 
 * messages are printed accordinly.
 =================================================================================
 */

#ifndef P2_H
#define P2_H

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include "getword.h"

//Static globals
#define MAXIMUM 100
#define MAX_STORAGE 25500

//=================================================================================
//Mutible globals
char outFile[STORAGE];
char inputFile[STORAGE];
//for args in word_args
int argCount = 0;
//flag to clear stdin for next iteration
int clearStdinFlag = 0;
//communicates with getword.c
int EOL_REACHED;
int backSlash;


//=================================================================================
//error codes
#define PIPE_ERROR                  -50
#define MAX_WORD_EXCEEDED           -49
#define CD_FAIL                     -48
#define AMBIGUOUS_REDIRECT_IO       -47
#define NO_DIRECTORY_FOUND          -46
#define PERMISSION_DENIED           -45
#define INVALID_AMP_SYNTAX          -44
#define FORK_FAILED                 -43
#define FILE_ALREADY_EXISTS         -42
#define FILE_NOT_FOUND              -41
#define COULD_NOT_CREATE_FILE       -40
#define REDIRECT_FAILED             -39
#define INPUT_FILE_ERROR            -38
#define INPUT_REDIRECT_ERROR        -37
#define EXEC_COMMAND_FAILED         -36   
#define FAILED_TO_PIPE              -35
#define PIPE_REDIRECT_FAILED        -34
#define PIPE_LEFTHAND_FAILED        -33
#define PIPE_LEFTHAND_EXECUTE       -32
#define PIPE_RIGHTHAND_FAILED       -31
#define PIPE_RIGHTHAND_EXECUTE      -30
#define EXEC_CHILD_COMMAND_FAILED   -29
#define INVALID_REDIRECT_IN         -28
#define TOO_MANY_CD_ARGS            -27
#define MISSING_REDIRECT            -26
#define INVALID_REDIRECT_OUT        -25

#define SUCCESS                 1
#define EXIT                    1
#define TRUE                    1
#define FALSE                   0



//=================================================================================
//global flags
int pipe_set = 0;
int redirect_in = 0;
int redirect_out = 0;
int ambiguous_redirect = 0;
int background_proc = 0;

//=================================================================================
//function decs


/*
* Parses function by calling getword() and syntax checking each word. If any meta characters are found, set flags
* but don't add them to command array. If redirect flags are set, look to populate inputfile and outputfiles respectively
* and don't add them to the command array. Return wordCount/exitCode
*/
int parse(char** commands, char* buffer);

/*
* Sets up file redirection. It occurs inside of the child process so as to properly set the process's stdin 
* and stdout. Checks inputfile and outfile for valid file names, error handles overwriting and permission denied.
* Dissolves competition for keyboard input and redirects to /dev/null to avoid deadlock
*/
int redirect(void);

/*
* Sets up redirection (without call to redirect()) and executes a vertical pipe. Handles dup2() redirect errors
* and prints to stderr. Fairly straightforward function, pipe error handling is taken care of in parse()
*/
void exec_pipe(char** commands);

/*
* Built-in function for the cd command. Impliments chdir(). Checks arg count, throws error if arg count exceeds cd
* parameter limit. Checks for no args, if no args are found, cd to $HOME. Otherwise cd to word_args[1] (path)
*/
int cdHandle(char** commands);

/*
* Function to terminate program. Sends kill signal to group process to gracefully exit without trailing zombies.
* terminates with standard message.
*/
void exitHandle(void);

/*
* Function to handle errors. Very straightforward switch() handling the plethora of error messages.
*/
void errorHandle(int error);

/*
* Resets all flags, as well as sets all char arrays/pointers to null.
* It includes a while loop to manually flush stdin due to the behavior exhibited in getword().c
* in which a newline character is put back into stdin. Also handles cases where a command
* terminates early due to a syntax error, leaving the rest of the commandline still in stdin waiting to be parsed
* by getword(). The while loop simply looks to see if getword() ended with a 0 return. If not, getword() until 0
* is returned.
*/
void resetAll(char** commands, char* buffer);

/*
* Standard signal handler for the signal function. Print out specified error sent.
*/
void sighandler(int signum);

/*
* Prepares to execute command, creates the child process and then calls exec_child_command to handle the rest
* if background process is set, it doesn't wait for child to finish and instead prints out the process id
* in the given format
*/
void exec_command(char** commands);

/*
* Calls redirect() to prepare any necessary redirection. Looks to call pipe if pipe flag is set. If no pipe flag,
* execvp() the command and exit to parent. I originally had these exec_ methods return an int, but later realized
* it was pointless when exiting and calling perror();
*/
void exec_child_command(char** commands);

#endif
