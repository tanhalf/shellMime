#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>

//global array for forking with &
int last_exec_status = 0;
pid_t *pid_list; 
int p_index = 0;
int size = 0;
char ***list = NULL;
void handleExit(pid_t p);
void executeOutputOp(char *program, char *output, char **args);
void executeInputOp(char *program, char *input, char **args);
void executeAndOp(char*cmdSeg);
void addFork(char**args);
void forkIt();
int checkDelim(char*cmdLine);
int isProgram(char* line);
char* findcmd(char* cmd);
char* trim(char* line);

int main(int argc, char *argv[]) {
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    FILE *batch;
    FILE *pointer;
    list = malloc(sizeof(char**) * 1);

    if (argc == 2){ 
        batch = fopen(argv[1], "r"); 
        if (!batch) {
            perror("no such file exists");
            exit(1);
        }
    }
    
    if(argc < 2){ 
        printf("shellMime> "); 
        fflush(stdout); 
        pointer = stdin;
    }
        
    else if(argc > 2){
        fprintf(stderr, "Usage: shellMime [BATCHFILE]\n");
        exit(1);
    }
    else{
        pointer = batch;
    }
        while((read = getline(&line, &len, pointer)) != -1){
            if(argc < 2){ 
            printf("shellMime> "); 
            fflush(stdout);
            } 
            line[strcspn(line, "\r\n")] = '\0';
            line[strcspn(line, "#")] = '\0';
            if(read == -1 || line == NULL)
                break;
            if(*line == '\0')
                continue;
            char *ptr = line;
            char *cmd;
        while ((cmd = strsep(&ptr, ";\n")) != NULL){
            p_index =0;
            if (*cmd == '\0'){
                continue;
            }
            else if(cmd == NULL){
                exit(1);
            }
            char *copy = strdup(cmd);
            while (*copy == ' ') 
                copy++;
            if (strcmp(copy, "exit") == 0){
                exit(0);
            }
            else if(strstr(copy, "exit ")){
                fprintf(stderr, "Usage: cannot use exit with arguments");
                exit(0);
            }
            //printf("cmd is %s\n", copy);
            last_exec_status = checkDelim(copy);
            //printf("made it back to main!\n");
            forkIt();
            for (int i = 0; i < size; i++) {
                free(list[i]);
            }
            free(list);
            list = malloc(sizeof(char**) * 1);
            for(int i = 0; i < size; i++) {
                handleExit(pid_list[i]); 
            }
            
            size = 0;

            if (last_exec_status == 1){
                exit(1);
            }
        }
        }
    exit(0);
}

int checkDelim(char *cmdLine){
    //printf("made it to checkdelim: %s\n\n", cmdLine);
    if(cmdLine == NULL)
        return last_exec_status;
    char *copy = strdup(cmdLine);
    char *ptr = copy;
    char **args = malloc(sizeof(char*) * 2); 
    char *delim = strpbrk(ptr, "&<>");
    //printf("delim: %s\n", delim);

    args[0] = NULL;
    args[1] = NULL;

    if(delim == NULL){
        //printf("made it to delim = NULL: %s\n", ptr);
        
        executeAndOp(trim(ptr));
        //free(args);
        //free(copy);
        return last_exec_status=0;
    }
    else if(*delim == '&'){
        //printf("made it to executeAndOps: %s\n\n", ptr);
        char *trimPtr = trim(strsep(&ptr, "&"));
        executeAndOp(trimPtr);
        //free(args);
        checkDelim(ptr);
    }
    else if(*delim == '<'){
        char *program = strsep(&copy, " < ");
        char *inputFile = strsep(&copy, "&<> ");
        executeInputOp(program, inputFile,args); 
        free(args);
        //return last_exec_status;
    }
    else if(*delim == '>'){
        char *program = strsep(&ptr, ">");
        char *outputFile = trim(ptr);
        executeOutputOp(program, outputFile, args);
        return last_exec_status;
    }
    return last_exec_status;

}
void addFork(char**args){
    if(last_exec_status == -1)
        return;
    int args_size =0;
    while(args[args_size] != NULL){
        args_size++;
    }
    //printf("made it to addFork \n");
    char** temp_args = malloc(sizeof(char*) * (args_size + 1));
    list = realloc(list, sizeof(char**) * (size + 1));
    for(int i = 0; i < args_size; i++){
        temp_args[i] = strdup(args[i]);
        //printf("made it to addFork, size: %s\n", temp_args[i]);
    }
    list[size] = temp_args;
    size++;
    return;
    
}

void forkIt() {
    int failed = 0;
    //printf("fork called\n\n");
    pid_list = malloc(sizeof(pid_t) * size);
    
    for(int i = 0; i < size; i++){
        pid_t p = fork();
        if (p == 0){
            execv(list[i][0], list[i]);
            exit(0);
        }
        else{
            pid_list[failed] = p;
            failed++;
        }
    }
}

void executeOutputOp(char *program, char *output, char **args){
    args[0] = program;
    pid_t p = fork();
    if(p == 0){ 
        int fd = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(fd < 0){
            perror("opening output file failed");
            exit(1);
        }
        dup2(fd, STDOUT_FILENO); 
        close(fd);
        executeAndOp(trim(program));
        forkIt();
        for(int i = 0; i < size; i++) {
            wait(NULL);
        }
        
        exit(0);
    }
    else if (p > 0) {
        waitpid(p, NULL, 0);
        last_exec_status = -1;
        size = 0;
    }
}

void executeInputOp(char *program, char *input, char **args){
    args[0] = program;
    pid_t p = fork();
    size = 0;
    list = NULL;
    if(p == 0){ 
        int fd = open(input, O_RDONLY);
        if(fd < 0){
            perror("opening input file failed");
            exit(1);
        }
        dup2(fd, 0); 
        close(fd);
        execv(args[0], args);
        perror("execv for executeInputOp failed");
        exit(1);
    }
    else if(p>0){
        handleExit(p);
    }
}

void executeAndOp(char*cmdSeg){
    
    size_t cap = 0;
    char **args = malloc(sizeof(char*) * 2);
    args[0] = NULL;
    args[1] = NULL;
    char *copy = strdup(cmdSeg);
    char *pro = strsep(&copy, " &<>");
    //printf("inside AndOp func:copy: %s\n", copy);
    //printf("inside AndOp func:pro:  %s\n", pro);
    if(isProgram(pro)==1){
        args[0] = strdup(pro);
        free(pro);
    }
    else{
        args[0] = findcmd(pro);
        if(args[0] == NULL){
            last_exec_status = 1;
            free(args);
            free(pro);
            return;
        }
    }
        char *token;
        cap = 1;
        while((token = strsep(&copy, " &<>")) != NULL){
            if(*token == '\0')
                continue;
            args = realloc(args, sizeof(char*) * (cap + 2));
            args[cap] = token; 
            cap++;
        }
        args[cap] = NULL;
        free(copy);
        addFork(args); //dont fork yet, add to a list for forking
        free(args);

    return;
}

int isProgram(char* line){
    if(strpbrk(line, "/"))
        return 1;
    return 0;
}

char* trim(char* line){
    char *new;
    char *ptr = line;
    int count = 0;
    int bcount;

    while(*ptr == ' ')
        ptr++;
    
    while(*ptr != '\0' && *ptr != '\n' && *ptr != '\r'){
        count++;
        ptr++;
    }

    while(*ptr == ' '){
        count--;
        ptr--;
    }
    
    bcount = count;
    
    while(bcount != 0){
        ptr--;
        bcount--;
    }

    new = malloc((sizeof(char)) * (count+1));
    
    //printf("trim result: %s", ptr);
   for(int i = 0; i < count; i++){
        new[i] = *ptr;
        ptr++;
   }
   
   new[count] = '\0';
   //printf("trim result after for loop: %s.\n\n", new);
    return new;
}

char* findcmd(char* cmd){
    char *path = strdup(getenv("PATH"));
    char *dir;
    size_t len;
    char *checking = NULL;
    
    while((dir = strsep(&path, ":")) != NULL){
        len = strlen(dir) + strlen(cmd) + 2;
        checking = malloc(len);
        snprintf(checking, len, "%s/%s", dir, cmd);

        if(access(checking,F_OK)==0 && (access(checking,X_OK))==0)
            return checking;
    }
    free(checking);
    return NULL;
}

void handleExit(pid_t p){
    int status;
    waitpid(p, &status, 0);
    if (WIFEXITED(status)) {
        last_exec_status = WEXITSTATUS(status); // Save it!
    } else {
        last_exec_status = 1; // It crashed/failed
        
    }
}
