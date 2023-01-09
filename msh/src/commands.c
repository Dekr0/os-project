#include <sys/wait.h>

#include "commands.h"
#include "handler.h"
#include "utils.h"


// Hardcoded information for each command
static const struct CommandDB commands[NCMD] = {
    {"cdir", 1, REQUIRE},
    {"pdir", 0, OPTIONAL},
    {"lstasks", 0, OPTIONAL},
    {"run", 5, OPTIONAL},
    {"stop", 1, REQUIRE},
    {"continue", 1, REQUIRE},
    {"terminate", 1, REQUIRE},
    {"check", 1, REQUIRE},
    {"exit", 0, OPTIONAL},
    {"quit", 0, OPTIONAL}
};

void cdir (char *path) {

    if ( strlen(path) == 0 )
        return;

    char folders[MAXNTOKENS][MAXCHARS]; // Array of folders name in a path
    char envVars[MAXCHARS][MAXCHARS]; // Array of enviromental variables 
    char pathParsed[MAXLEN]; // Path after parsing all the environmental variable
    
    // Zero out
    memset( &folders, 0, sizeof(folders) );
    memset( &envVars, 0, sizeof(envVars) );
    memset( &pathParsed, 0, sizeof(pathParsed) );

    // Split the path into each individual folder name "/"
    int numFolders = split( path, folders, "/" );
    for ( int i = 0; i < numFolders; i++ ) {
        
        /* 
         * A folder name is a pure environmental variable
         * or part of a folder name contains one or more 
         * environmental variables.
         */ 

        if ( strchr( folders[i], '$' ) ) {

            /*
             * Omit "$" from the environmental variable and replaced 
             * by "|&". "|" is to separate all the environmental 
             * variables as well as normal string. "&" is to identify
             * all the enviromental variables from other normal strings
             * 
             * Example: 
             * folders[i] = myproject$HOME$PWD
             * after gsub
             * folders[i] = myproject|&HOME|&PWD
             * after split
             * envVars = { myproject, &HOME, &PWD }
             */

            gsub( folders[i], "$", "|&" );
            int numEnvVar = split( folders[i], envVars, "|" );

            memset(folders[i], 0, sizeof(folders[i]));

            // Replace each environmental variable with its value
            for ( int j = 0; j < numEnvVar; j++) {
                
                if ( startWith( envVars[j], '&' ) ) {
                    char *envVal = NULL;
                    size_t bufferSize;
                    
                    envVal = pathAlloc(&bufferSize);
                    
                    gsub( envVars[j], "&", "" );
                    strcpy( envVal, envVars[j] );
                    if ( envVal != NULL && xgetenv(envVal) != 0) {
                        memset( envVars[j], 0, sizeof(envVars[j]) );
                        strcpy( envVars[j], envVal );
                    }

                    free(envVal);
                } 
                
                // Concatenate each environmental value and normal string to 
                // form a folder name    
                strcat( folders[i], envVars[j] );
            }
        }

        // Concatenate each folder name to get a path
        strcat( pathParsed, folders[i] );
        strcat( pathParsed, "/" );
    }
    
    // If a path is a relative path, attach the absolute path of the current 
    // directory 
    if ( !startWith(pathParsed, '/' ) ) {
        char *cwd = NULL;

        cwd = xgetcwd();

        if ( cwd != NULL ) {
            strcat( cwd, "/" );
            strcat( cwd, pathParsed );
            strcpy( pathParsed, cwd );
        }
    }

    if ( chdir(pathParsed) < 0 )
        warning( "cdir: %s: No such file or directory\n", pathParsed );
    else 
        printf( "cdir: done (pathname=%2s)\n", pathParsed );
}

void check (char *targetId) {
    int numPid;
    int status;
    int targetPid;
    FILE *file;

    // buffer used for split function, prevent split modified the input string
    char buf[MAXBUF]; 
    char line[MAXBUF];
    // a list of pids of processes that are child of a parent process
    int *pidList = NULL; 
    // Information of a process after split a line from input of ps
    // It contains : user, pid, ppid and state
    char tokens[4][MAXCHARS];

    numPid = 0;
    targetPid = atoi(targetId);

    pidList = (int *)malloc( 128 * sizeof(int) );

    // zero out
    memset( &buf, 0, sizeof(buf) );
    memset( &line, 0, sizeof(line) );
    memset( &tokens, 0, sizeof(tokens) );

    file = popen( "ps -u $USER -o user,pid,ppid,state,start,cmd --sort start", "r" );
    if (file) {
        while ( fgets( line, MAXBUF, file ) != NULL ) {
            if ( strstr( line, "USER" ) ) {
                printf( "%s", line );
                continue;
            }

            // omit process of ps if target id of the process is msh379
            if ( strstr( line, "ps -u" ) ) continue;

            // copy input line into split buffer
            strcpy( buf, line );
            
            split( buf, tokens, " " );
            int pid = atoi(tokens[1]);
            int ppid = atoi(tokens[2]);
            char *state = tokens[3];

            if ( pid == targetPid ) {
                printf( "%s", line );
                // process is not running
                if ( strcmp( state, "Z" ) == 0 ) 
                    break;
        
                pidList[numPid] = pid;
                numPid++;
                continue;
            }

            // That ppid of a process is equal to the target pid means
            // this process is a child process
            if ( ppid == targetPid ) {
                printf( "%s", line );
                
                pidList[numPid] = pid;
                numPid++;
                continue;
            }

            // If ppid of a process is in a list of pids of processes that are 
            // child of a parent process, this process is a descendant process.
            if ( isChildPs( ppid, pidList, numPid ) ) {
                printf( "%s", line );

                pidList[numPid] = pid;
                numPid++;
                continue;
            }
        }
    }

    status = pclose(file);
    if ( status == -1 )
        fatal( "check: cannot close the pipe\n" );

    free(pidList);
}

int checkArgs ( int cmdIndex, int numArgs ) {
    if ( commands[cmdIndex].optional )
        // There are no hard requirement for number of arguments for this command
        return 1;
    
    if ( numArgs > commands[cmdIndex].numArgs ) {
        warning( "%s: too many arugments\n", commands[cmdIndex].name );
        return 0;
    }

    if ( numArgs < commands[cmdIndex].numArgs ) {
        warning( "%s: not enought arguments\n", commands[cmdIndex].name );
        return 0;
    }

    return 1;
}

int getTaskNo ( char *cmdName, char *strTaskNo, int numTasks, struct TaskDB *taskList ) {
    int taskNo = -1;
 
    if ( numTasks == 0 ) {
        warning( "%s: No tasks are running\n", cmdName );
        return taskNo;
    }

    taskNo = atoi(strTaskNo);
    if ( taskNo < 0 || taskNo > NTASK || 
        (taskList[taskNo].pid == -1 && taskList[taskNo].running == -1) ) {
        warning( "%s: Invalid taskNo\n", cmdName );
        taskNo = -1;
    }

    return taskNo;
}

int getcmdIndex (char *command) {
    int cmdIndex = -1;

    for (int i = 0; i < NCMD; i++) {
        if ( strcmp( commands[i].name, command ) == 0 ) {
            cmdIndex = i;
            break;
        }
    }

    return cmdIndex;
}

int isChildPs ( int ppid, int pidList[], int numPid ) {
    for ( int i = 0; i < numPid; i++ ) {
        if ( pidList[i] == ppid ) return 1;
    }

    return 0;
}

int isZombie (pid_t pid) {
    int status = 0;

    sleep(1);
    
    if ( waitpid( pid, &status, WNOHANG ) == pid ) {
        warning( "run: new process is terminated immedately after created\n" );
        warning( "run: process %d is marked as terminated\n", pid );
        return 1;
    }

    return 0;
}

void lstasks (struct TaskDB *taskList, int numTasks) {
    for ( int i = 0; i < numTasks; i++ ) {
        if (taskList[i].running != -1)
            printf( "%d: (pid= %d, cmd= %s)\n", taskList[i].index, taskList[i].pid, taskList[i].command );
    }
}

void pdir () {
    char *cwd = NULL;
    
    // Call xgetcwd() to get the path of current directory
    cwd = xgetcwd();

    if ( cwd != NULL )
        printf( "%s\n", cwd );

    free(cwd);
}

pid_t run ( char *pgm, char args[][MAXCHARS], int numArgs ) {
    pid_t pid;

    if ( ( pid = fork() ) < 0 )
        warning( "run: fork error\n" );

    if ( pid > 0 )
        printf( "run: new process is created\n" );

    if ( pid == 0 && strlen(pgm) != 0 ) {
        char *argv[NARG];
        
        argv[0] = pgm;

        // Construct an array of arugments for execvp
        for (int i = 0 ; i <= numArgs; i++ )
            argv[i+1] = args[i+2];

        // null pointer for arguments
        argv[numArgs+1] = NULL;

        if ( execvp( pgm, argv ) < 0 ) {
            warning( "run: cannot run %s\n", pgm );
            exit(EXIT_FAILURE);
        }
    }

    return pid;
}

int stop ( pid_t pid ) {
    int status = 0;

    if ( kill( pid, SIGSTOP ) == -1)
        fatal( "stop: cannot stop task %d\n", pid );
    else {
        if ( waitpid( pid, &status, WUNTRACED ) != pid )
            fatal( "stop: cannot get status of task %d\n", pid );
        
        // using macro funciton WSTOPSIG to check status to ensure the process 
        // truly stopped
        if ( !WSTOPSIG(status) ) {
            fatal( "stop: cannot stop task %d\n", pid );
            status = 0;
        } else {
            printf( "stop: task %d stopped\n", pid );
            status = 1;
        }
    }

    return status;
}

void terminate ( pid_t pid ) {
    if ( kill( pid, SIGKILL ) == -1 )
        fatal( "terminate: cannot terminate task %d normally", pid );
    else {
        int status;
        if ( waitpid( pid, &status, WUNTRACED ) != pid )
            fatal( "terminate: cannot get status of task %d\n", pid );
        
        printf( "task %d terminated\n", pid );
    }
}

int xcontinue ( pid_t pid ) {
    int status = 0;

    if ( kill( pid, SIGCONT ) == -1 )
        fatal( "continue: cannot continue task %d\n", pid );
    else {
        // enable status check after send SIGCONT by using WCONTINUED option 
        if ( waitpid( pid, &status, WCONTINUED ) != pid ) 
            fatal( "continue: cannot get status of task %d\n", pid );
        
        // using macro funciton WIFCONTINUED to check status to ensure the process 
        // truly continue
        if ( !WIFCONTINUED(status) ) {
            fatal( "continue: cannot continue task %d\n", pid );
            status = 0;
        } else {
            printf( "continue: task %d is resumed\n", pid );
            status = 1;
        }
    }

    return status;
}

void xexit ( clock_t startWallTime, int numTasks, struct tms *tmsStart, struct TaskDB *taskList ) {
    // terminate all tasks that are not terminated
    if ( taskList != NULL ) {
        for ( int i = 0; i < numTasks; i++ ) {
            if ( taskList[i].running != -1 )
                terminate(taskList[i].pid);
        }
    }

    // end time recording and calculate time
    struct tms tmsEnd;
    memset( &tmsEnd, 0, sizeof(tmsEnd) );

    clock_t endWallTime = times(&tmsEnd);
    if ( endWallTime == -1 )
        warning( "cannot end recording user CPU time\n" );

    printf("\n");
    printTime(endWallTime - startWallTime, tmsStart, &tmsEnd);
}

char *xgetcwd () {
    char *cwd = NULL;
    size_t bufferSize;

    // allocate memory for the path buffer
    cwd = pathAlloc(&bufferSize);

    if ( cwd == NULL ) {
        warning( "xgetcwd: cannot allocate memory for path string\n" );
    }

    if ( getcwd( cwd, bufferSize ) == NULL ) {
        warning( "xgetcwd:cannot get current directory\n" );
    }

    return cwd;
}

int xgetenv (char *envVar) {
    char *path = NULL;
    
    // get the value of an environemntal variable
    path = getenv(envVar);

    if (path == NULL)
        return 0;

    memset( envVar, 0, sizeof(envVar) );
    strcpy( envVar, path );

    return 1;
}
