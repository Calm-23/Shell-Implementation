#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

int bg = 0;
char **tokenize(char *line){
        char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
        char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
        int i, tokenIndex = 0, tokenNo = 0;

        for(i =0; i < strlen(line); i++){

                char readChar = line[i];                                                                                                                        
                if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
                        token[tokenIndex] = '\0';
                        if (tokenIndex != 0){
                                tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
                                strcpy(tokens[tokenNo++], token);
                                tokenIndex = 0;
                        }
                } else {
                        token[tokenIndex++] = readChar;
                }
        }

        free(token);
        tokens[tokenNo] = NULL ;
        return tokens;
}
static void piping(char ***com){
        int fds[2];
        pid_t pid;
        int ftemp = 0;

        while (*com){
                pipe(fds);
                if((pid = fork()) == -1){
                        perror("fork");
                        exit(1);
                }
                else if(pid == 0){
                        dup2(ftemp, 0);
                        if(*(com + 1) != NULL)
                                dup2(fds[1], 1);
                        close(fds[0]);
                        execvp((*com)[0], *com);
                        exit(1);
                }
                else{
                        if(bg==0) wait(NULL);
                        close(fds[1]);
                        ftemp = fds[0];
                        com++;
                }
        }
}

void run(int argc, char** arg)
{
        pid_t pid;
        const char* amp;
        amp = "&";

        if (strcmp(arg[argc - 1], amp) == 0)
                bg = 1;

        pid = fork();
        if (pid < 0)
                perror("Error (pid < 0)");

        else if (pid == 0) {

                if (bg==1) {
                        arg[argc - 1] = NULL;
                        argc--;
                }

                execvp(arg[0], arg);
                perror("execvp error");
        }
        else if (bg==0)
                waitpid(pid, NULL, 0);
}

void ls()
{
        DIR* d;
        struct dirent* dir;
        d = opendir(".");
        if (d != NULL) {
                while ((dir = readdir(d)) != NULL) {
                        printf("%s\t", dir->d_name);
                }
                closedir(d);
        }
        else
                printf("Error!");
}

void pwd()
{
        char add[1024];
        if (getcwd(add, sizeof(add)))
                printf("%s", add);
        else
                printf("Error");
                }

void cd(char* add)
{
        if (chdir(add))
                printf("No such file and directory");
}

int main(int argc, char* argv[]) {
        char  line[MAX_INPUT_SIZE];
        char  **tokens;
        int i;

        FILE* fp;
        if(argc == 2) {
                fp = fopen(argv[1],"r");
                if(fp < 0) {
                        printf("File doesn't exists.");
                        return -1;
                }
        }

        while(1) {
                printf("\n");
                bzero(line, sizeof(line));
                if(argc == 2) {
                        if(fgets(line, sizeof(line), fp) == NULL) break;
                        line[strlen(line) - 1] = '\0';
                }
                else {
                        printf("[");
                        pwd();
                        printf("]$ ");
                        scanf("%[^\n]", line);
                        getchar();
                }
                line[strlen(line)] = '\n';
                tokens = tokenize(line);

                int len = 0;
                for (i = 0; tokens[i] != NULL; i++)
                        len++;
                if (strcmp("ls", tokens[0]) == 0)
                        ls();
                else if (strcmp("pwd", tokens[0]) == 0)
                        pwd();
                else if (strcmp("cd", tokens[0]) == 0)
                        cd(tokens[1]);
                else if (strcmp("exit", tokens[0]) == 0)
                        exit(0);
                else{
                         int pcount=0, k;
                        for(k=0; tokens[k]; k++){
                                if(strcmp(tokens[k], "|")==0) pcount++;
                        }
                        if(pcount == 0)
                                run(len, tokens);
                        else{
                        int l=0, j, count=0, totalPipes=pcount+2;
                        
                        char **com_string= (char**)malloc(MAX_NUM_TOKENS * sizeof(char*));
                        char ***com = (char***)malloc(totalPipes * sizeof(char**));

                        for(j=0; j<len; j++){
                                if(strcmp("|", tokens[j]) == 0){
                                        com_string[count] = NULL;
                                        com[l] = com_string;
                                        count = 0;
                                        com_string = (char**)malloc(MAX_NUM_TOKENS * sizeof(char*));
                                        l++;
                                }
                                else 
                                        com_string[count++] = tokens[j];
                        }

                        com[l++] = com_string;
                        com[l] = NULL;

                        piping(com);
                   }
                }
                  for(i=0;tokens[i]!=NULL;i++){
                        free(tokens[i]);
                }
                free(tokens);
                bg = 0;
        }
        return 0;

}