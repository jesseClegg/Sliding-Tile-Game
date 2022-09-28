/*
 * Representing a "sliding puzzle" game
 * Uses C99 standard
 * Tested and compiled on Apple clang version 12.0.0 (clang-1200.0.32.28)
 * Tests run on x86_64-apple-darwin20.2.0
 * @author Jesse Clegg
 * @version 3.0
 */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>


void clientFunction(int *command, int *data);

void serverFunction(int *command, int *data);

int main() {
    int commandPipe[2];
    int dataPipe[2];
    pid_t client;
    if (pipe(commandPipe)) {
        perror("pipe one failed");
        exit(1);
    }
    if (pipe(dataPipe)) {
        perror("pipe two failed");
        exit(1);
    }
    client = fork();
    if (client == -1) {
        perror("fork one failed");
        exit(EXIT_FAILURE);
    }
    if (client == 0) {
        clientFunction(commandPipe, dataPipe);//pass the pipes in, globals are bad
    } else {
        serverFunction(commandPipe, dataPipe);//pass the pipes in, globals are bad
        wait(NULL);
    }
}