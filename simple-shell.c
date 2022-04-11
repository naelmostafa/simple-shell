#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define MAX_STRING 1028


//Welcome message
void init(){
    system("clear");
    fflush(stdin);
    printf("\n\n\n"
    "****************************************\n"
    "*       welcome to simple-shell         *\n"
    "****************************************\n\n\n");
}

// read input from user
void readInput(char *str){
    char buffer[MAX_STRING];
    fgets(buffer, MAX_STRING, stdin);
    strcpy(str, buffer);
}

//print working directory using getcwd()
void pwd(){
    char cwd[MAX_STRING];

    if(getcwd(cwd, sizeof(cwd)) != NULL){

        printf("\nsimple-shell-%s-$->",cwd);
    }else{

        printf("error getcwd()");
        exit(0);
    }
}

void logFile(pid_t pid, char** parsed){
    FILE *file = fopen("log.txt" ,"a");
    if(file == NULL){
        printf("file error");
    }else{
        fprintf(file, "+[%d] %s %s\n", pid, parsed[0], parsed[1]);
        fclose(file);
    }

}

//parse whiteSpace
void parse(char *str, char **parsed){
    strtok(str,"\n");                       //remove the newline
    for(int i=0; i<MAX_STRING; i++){
        parsed[i] = strsep(&str, " ");      //strsep to separate words

        if(parsed[i] == NULL)
            break;
        //ignore extra space between cmd and arguments
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

//Parse &
int checkForSpecial(char *str){
    for(int i=0 ;i<MAX_STRING; i++){    //Check for &
        if(str[i] == '&'){
            strtok(str,"&");
            return 1;
        }

        if(str[i] == '\0')
            break;
    }
    return 0;
}

//signal Handler
void signalHandler(int signum){
    wait(NULL);
}
// process run in foreground flag = 0
pid_t execute(char** parsed){
        int status;
        pid_t pid = fork(); // create a child

        if(pid < 0 ){
            printf("Failed Fork\n");
        }else if(pid ==0 ){
            if(execvp(parsed[0], parsed)< 0)
                printf("%s :command not found",parsed[0]);
            exit(0);

        }else{
            wait(&status); // wait until the process is terminated
            logFile(pid ,parsed);
            return pid;
        }
}
// process run in the background flag = 1
pid_t executeBack(char** parsed){

        pid_t pid = fork();

        if(pid < 0 ){
            printf("Faild Fork\n");
        }else if(pid == 0){
            if(execvp(parsed[0], parsed)< 0)
                printf("%s :command not found",parsed[0]);

            exit(0);
        }else{
            signal(SIGCHLD, signalHandler);      //Handle zombies
            logFile(pid ,parsed);
            return pid;
        }
}



int main(){
    char input[MAX_STRING];
    // commands ==> parsed[0], arguments ==> parsed[1]
    char *parsed[MAX_STRING];
    int flag = 0;   //  & flag
    // create and overwrite existing file
    FILE *file;
    file = fopen("log.txt" ,"w");
    fclose(file);

    init();         //Print the welcome message

    while(1){
        pid_t pid = getpid();

        pwd();
        readInput(input);

        if(input[0] == '\n' || input[0] == '\0'){ //check for empty input
           continue;
        }

        flag = checkForSpecial(input);

        parse(input, parsed);

        if(strcmp(input, "exit") == 0) {     //exit shell
            exit(0);
        }
        if( strcmp(parsed[0], "cd") == 0 ){
            if(chdir(parsed[1]) != 0){      // CD
                perror(parsed[1]);          //if failed print error
            }
            continue;
        }
        if(flag == 0){                      // normal execution
            pid = execute(parsed);

        }else if(flag == 1){                // & exist
            pid = executeBack(parsed);
        }

        fflush(stdin);                      //clear Output Buffer
    }

    return 0;
}
