#include <signal.h>

#include <sys/wait.h>

#include "commands.h"
#include "handler.h"
#include "utils.h"


// Notice that taskNo start from 0

int main () {

  /* --------- Start time recording --------- */
    struct tms tmsStart;
    memset( &tmsStart, 0, sizeof(tmsStart) );
    
    clock_t startWallTime = times(&tmsStart);
    if ( startWallTime == -1 )
        warning( "Fail to start recording user CPU time" );


 /* --------- Set CPU time limit --------- */
    const struct rlimit limit = {
        300, // Soft limit
        600 // Hard limit
    };

    if ( setrlimit( RLIMIT_CPU, &limit ) != 0 )
        warning( "Fail to set CPU time limit" );

 /* --------- Variables Initialization --------- */
    
    int numTasks; // number of tasks msh has (limit 32)
    pid_t parent_pid; // pid of msh

    char inStr[MAXLEN]; // buffer for reading input command
    char tokens[MAXNTOKENS][MAXCHARS]; // buffer for storing command name and args
    struct TaskDB taskList[NTASK]; // information of all tasks under msh

    
    memset( &inStr, 0, sizeof(inStr) );
    memset( &tokens, 0, sizeof(tokens) );
    
    for ( int i = 0; i < NTASK; i++ ) {
        taskList[i].index = -1;
        taskList[i].pid = -1;
        taskList[i].running = -1;
        memset( taskList[i].command, 0, sizeof(0) );
    }

    numTasks = 0;
    parent_pid = getpid();


 /* --------- Main Loop --------- */
    printf( "msh [%d]: ", parent_pid );
    while( scanf("%[^\n]", inStr) != EOF) {
        getchar();

        // split the input and get the command name and argument
        int numTokens = split( inStr, tokens, " " );

        if ( numTokens == 0 ) { 
            printf( "msh [%d]: ", parent_pid );
            continue;
        }

        char *command = tokens[0];

        // get the switch statement index of a command 
        int cmdIndex = getcmdIndex( command ); 

        if ( cmdIndex == -1 ) {
            warning( "msh: %s: command not found\n" );
            printf( "msh [%d]: ", parent_pid );
            memset( inStr, 0, sizeof(inStr) );
            memset( tokens, 0, sizeof(tokens) );
            continue;
        }
        
        // validate the number of arguments
        if ( checkArgs( cmdIndex, numTokens - 1 ) == 0 ) {
            printf( "msh [%d]: ", parent_pid );
            memset( inStr, 0, sizeof(inStr) );
            memset( tokens, 0, sizeof(tokens) );
            continue;
        }

        switch(cmdIndex) {
            case 0: // cdir
                cdir(tokens[1]); break;
            case 1: // pdir
                pdir(); break;
            case 2: // lstasks
                lstasks(taskList, numTasks); break;
            case 3: // run
                if (numTasks == NTASK) {
                    warning( "run: number of task limit reached, cannot run more \
                     task\n" ); 
                    break;
                }

                if ( numTokens > 2 && numTokens > 6 ) {
                    warning( "run: too many arguments for program %s\n", 
                    tokens[1] ); 
                    break;
                }

                pid_t pid;

                // fork a process and launch a specified program
                pid = run( tokens[1], tokens, numTokens - 2 );
                
                // debugging use, trigger if run is executed without specified pgm
                if ( pid == 0 ) return 0;

                if ( pid > 0 ) {
                    char command[MAXCHARS];
                    memset( command, 0, sizeof(command) );

                    // reconstruct input command line after split
                    for ( int i = 1; i < numTokens; i++ ) {
                        strcat( command, " " );
                        strcat( command, tokens[i] );
                    }

                
                    // assign information of a newly create task
                    taskList[numTasks].index = numTasks;
                    taskList[numTasks].pid = pid;

                    // mark the process as terminated if it terminates right 
                    // after the process is created because of errors / exceptions
                    if ( isZombie(pid) ) 
                        taskList[numTasks].running = -1;
                    else
                        taskList[numTasks].running = 1;
                    strcpy( taskList[numTasks].command, command );
                    numTasks++;
                }

                break;
            case 4: { // stop
                int taskNo = getTaskNo( tokens[0], tokens[1], numTasks, taskList );
                // validate the provided taskNo and convert to pid_t
                // same for continue and terminate command

                // Invalid taskNo
                if ( taskNo == -1 ) break;
                
                if (taskList[taskNo].running == -1) {
                    warning( "stop: task %d is already terminated\n", taskNo );
                    break;
                }

                if (!taskList[taskNo].running) 
                    warning( "stop: task %d is already stopped\n", taskNo );
                else {
                    if(stop(taskList[taskNo].pid))  
                        taskList[taskNo].running = 0;
                    // The running is not marked as 0 if that process is not 
                    // stopped successfully
                }

                break;
            }
            case 5: { // continue
                int taskNo = getTaskNo( tokens[0], tokens[1], numTasks, taskList );
                
                if ( taskNo == -1 ) break;
                
                if ( taskList[taskNo].running == -1 ) {
                    warning( "continue: task %d is already terminated\n", taskNo );
                    break;
                }

                if (taskList[taskNo].running) 
                    warning( "continue: task %d is already running\n", taskNo );                    
                else { 
                    if (xcontinue(taskList[taskNo].pid))
                        taskList[taskNo].running = 1;
                        // The running is not marked as 1 if that process is not 
                        // stopped successfully
                }
                
                break;
            }
            case 6: { // terminate
                int taskNo = getTaskNo( tokens[0], tokens[1], numTasks, taskList );
                if ( taskNo == -1 ) break;
                
                if (taskList[taskNo].running == -1) {
                    warning( "terminate: task %d is already terminated\n", taskNo );
                    break;
                }

                terminate(taskList[taskNo].pid);

                // Reset the slot to unused state
                taskList[taskNo].running = -1;
                break;
            }
            case 7: // check
                check(tokens[1]);
                break;
            case 8: // exit (with clean up)
                xexit( startWallTime, numTasks, &tmsStart, taskList );
                return 0;
            case 9: // quit (without clean up)
                xexit( startWallTime, numTasks, &tmsStart, NULL );
                return 0;
        }

        printf( "msh [%d]: ", parent_pid );
        memset( inStr, 0, sizeof(inStr) );
        memset( tokens, 0, sizeof(tokens) );        
    }
}
