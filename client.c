#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rawSocketConnection.h"
#include "packet.h"
#include "fileHelper.h"
#include "backup.h"

#define ETHERNET "lo"
#define PATH_MAX 128

char* strdup(const char *c)
{
    char *dup = malloc(strlen(c) + 1);

    if (dup != NULL)
       strcpy(dup, c);

    return dup;
}

char** parseInput(char* input) {
    int capacity = 2;
    char** inputParsed = (char**) malloc(capacity * sizeof(char*));
    char* copy = strdup((const char*) input);
    char* token;
    token = strtok(copy, " ");
    int numberOfParts = 0;

    while (token != NULL) { 
        if (numberOfParts >= capacity) {
            capacity += 1;
            inputParsed = (char**) realloc(inputParsed, capacity * sizeof(char*));
        }
        inputParsed[numberOfParts] = strdup(token);
        numberOfParts++;
        token = strtok(NULL, " ");
    }

    free(copy);
    return inputParsed;
}

void changeDirectory(char* path) {
    if (chdir(path) != 0) {
        printf("Directory doesn't exist\n");
    }
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }
}

int main(int argc, char** argv) {
    int socket;
    socket = rawSocketConnection(ETHERNET);

    char* input = malloc(sizeof(char) * 100);
    char** inputParsed;
    char* command;

    // char* path = malloc(sizeof(char) * 100);

    while (1) {
        fgets(input, 100, stdin);
        input[strcspn(input, "\n")] = 0;
        inputParsed = parseInput(input);
        command = inputParsed[0];

        if (!(strcmp(command, "cd"))) {
            char* path = inputParsed[1];
            changeDirectory(path);
        }
        else if (!(strcmp(command, "backup"))) {
            char* fileName = inputParsed[1];
            makeBackup(socket, fileName);
        }
        else if (!(strcmp(command, "exit"))) {
            break;
        }
    }

    return 0;
}