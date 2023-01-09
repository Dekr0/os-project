#include <signal.h>

#include "utils.h"


#ifndef COMMANDS_H
#define COMMANDS_H
#define NCMD 10 // number of commands in this msh379
#define NARG 6
#define MAXBUF 1024
#define REQUIRE 0
#define OPTIONAL 1

/**
 * @brief A struct for representation of a command
 */
struct CommandDB {
    char name[16]; // name of the command
    int numArgs; // number of required arguments
    int optional; // arguments is optional or mandatory
};

/**
 * @brief Change the directory of process to the specified pathname
 * 
 * @param pathname absolute / relative path 
 */
void cdir (char *pathname);

/**
 * @brief Print the information of a process based on the target id
 *  from ps command
 * 
 * @param targetId target id of a process
 */
void check (char *targetId);

/**
 * @brief Check the number of arguments is valid for specific command provided
 * the switch index of command
 * 
 * @param cmdIndex switch index of the command
 * @param numArgs number of arguments provided from the input
 * @return int 1 is valid; 0 is valid;
 */
int checkArgs ( int cmdIndex, int numArgs );

/**
 * @brief Validate the provided taskNo is in the taskList
 * 
 * @param cmdName name of command (for display)
 * @param strTaskNo string form of the taskNo
 * @param numTasks number of tasks in the program (for iteration of the taskList)
 * @param taskList pointer to the taskList
 * @return int taskNo in integer form
 */
int getTaskNo ( char *cmdName, char *strTaskNo, int numTasks, struct TaskDB *taskList );

/**
 * @brief get switch index of the command
 * 
 * @param command name of the command
 * @return int 
 */
int getcmdIndex (char *command);

/**
 * @brief Check whether if a process is a grandchild process of a parent process
 * 
 * @param ppid ppid of a process readed from ps command
 * @param pidList a list of pids of processes that are child of a parent process
 * @param numPid number of pids in a pidList (for iteration of the pidList)
 * @return int 1 is a grandchild process; 0 is not
 */
int isChildPs ( int ppid, int pidList[], int numPid );

/**
 * @brief Check if a process is terminated or in zombie state
 * 
 * @param pid
 * @return int 1 is terminated; 0 is not terminated
 */
int isZombie (pid_t pid);

/**
 * @brief List all accepted tasks that have not been explicitly terminated by the user.
 * 
 * @param taskList pointer of taskList
 * @param numTasks 
 */
void lstasks (struct TaskDB *taskList, int numTasks);

/**
 * @brief Fork a process to run the program specified by pgm, using the given 
 * arguments (at most 4 arguments may be specified). A task is considered 
 * accepted if msh379 has successfully launched the process. msh379 accepts 
 * at most NTASK (32) tasks
 * 
 * @param pgm name of the program
 * @param args list of arguments
 * @param numArgs number of arguments
 * @return pid_t pid of the process
 */
pid_t run ( char *pgm, char args[][MAXCHARS], int numArgs );

/**
 * @brief Stop (temporarily) the task whose index is taskNo by sending signal SIGSTOP
 * to its head process.
 * 
 * @param pid  
 * @return int Return 1 if a process is successfully stopped. Otherwise, 0
 */
int stop ( pid_t pid );

/**
 * @brief Continue the execution of the task whose index is taskNo by sending signal 
 * SIGCONT to its head process.
 * 
 * @param pid 
 * @return int Return 1 if a process is successfully resumed. Otherwise, 0
 */
int xcontinue ( pid_t pid );

/**
 * @brief Terminate the execution of the task whose index is taskNo by sending
 * signal SIGKILL to its head process. Notice that, it's possible that terminate 
 * complains if it try to terminate a process that already in zombie state. 
 * However, this is still counted that the task is terminated in the task list.
 * 
 * @param pid 
 */
void terminate ( pid_t pid );

/**
 * @brief Print the current working directory.
 * 
 */
void pdir ();

/**
 * @brief A group of instructions before exit the program :
 *  Terminate every task in the taskList
 *  end the time recording
 *  calculate the user CPU time and other related time 
 * 
 * @param startWallTime 
 * @param numTasks 
 * @param tmsStart 
 * @param taskList 
 */
void xexit ( clock_t startWallTime, int numTasks, struct tms *tmsStart, struct TaskDB *taskList );

/**
 * @brief Get environmental values 
 * 
 * @param envVar 
 * @return int return 1 if a value is obtained. Otherwise, 0 
 */
int xgetenv (char *envVar);

/**
 * @brief Get the path of the current directory
 * 
 * @return char* path of the current directory
 */
char *xgetcwd ();

#endif
