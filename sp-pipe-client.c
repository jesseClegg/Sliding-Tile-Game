/*
 * Representing client side of "sliding puzzle" game
 * Uses C99 standard
 * Tested and compiled on Apple clang version 12.0.0 (clang-1200.0.32.28)
 * Tests run on x86_64-apple-darwin20.2.0
 * @author Jesse Clegg
 * @version 3.0
 */
#include <stdio.h>
#include <unistd.h>

enum menuOptions {
    print, save, load, new, move, noAction
};

/*
 * Client function acts as middleman for user and server, process user inputs and return results
 * Straightforward design pattern any reasonable developer should know
 * @param: command is the command pipe created in main function, accessible to both client and server
 * @param: data is the data pipe created in main function, accessible to both client and server
 */
void clientFunction(int *command, int *data) {
    int commandPipe[2];
    commandPipe[0] = *command;
    commandPipe[1] = *(command + 1);
    int dataPipe[2];
    dataPipe[0] = *data;
    dataPipe[1] = *(data + 1);
    close(commandPipe[0]);
    close(dataPipe[1]);
    enum menuOptions menuCommand;
    char userInput;
    int isWon = 0;
    int boardsize;
    char fileName[99];
    int success = 0;
    int newSize;

    while (1) {
        read(dataPipe[0], &isWon, sizeof(int));
        if (isWon == 1) {
            printf("YOU WON THE GAME!!!\n");
            printf("Starting a new game of default size...\n");
        }
        printf("Menu: [p]rint, [q]uit, [s]ave, [l]oad, [n]ew, [m]ove\n");
        fflush(stdin);//clear out anything left over
        scanf("%c", &userInput);
        if (userInput == 'p') {
            menuCommand = print;
            write(commandPipe[1], &menuCommand, sizeof(menuCommand));
            read(dataPipe[0], &boardsize, sizeof(boardsize));//need to find out the board size from server
            for (int i = 0; i < (boardsize * 4) + 1; i++) { //Top boundary that scales. (nice little touch for U.I.)
                printf("-");
            }
            printf("\n");
            for (int i = 0; i < boardsize; i++) {
                printf("|");
                for (int j = 0; j < boardsize; j++) {
                    int currentTileValue;
                    read(dataPipe[0], &currentTileValue, sizeof(int));//read over each character, synced up perfectly
                    if (currentTileValue == -1) {//no printing empty tile value
                        printf("%3c|", ' ');
                    } else {
                        printf("%3d|", currentTileValue);
                    }
                }
                printf("\n");
            }
            for (int i = 0; i < (boardsize * 4) + 1; i++) {//Matching bottom boundary.
                printf("-");
            }
            printf("\n");
        } else if (userInput == 'q') {
            printf("Quitting the game...\n");//don't need menu command for quite, not sending it over
            close(commandPipe[1]);//rely on closing pipe to notify server of quitting
            close(dataPipe[0]);
            break;
        } else if (userInput == 's') {
            menuCommand = save;
            write(commandPipe[1], &menuCommand, sizeof(menuCommand));
            printf("Enter file name to save..\n");
            scanf("%s", fileName);
            write(commandPipe[1], &fileName, sizeof(fileName));
            read(dataPipe[0], &success, sizeof(success));
            if (success == 1) {
                printf("File [%s] saved successfully\n", fileName);
            } else {
                printf("Failed to save file [%s]\n", fileName);
            }
        } else if (userInput == 'l') {
            menuCommand = load;
            write(commandPipe[1], &menuCommand, sizeof(menuCommand));
            printf("Enter file name to load..\n");
            scanf("%s", fileName);
            write(commandPipe[1], &fileName, sizeof(fileName));
            read(dataPipe[0], &success, sizeof(success));
            if (success == 1) {
                printf("File [%s] loaded successfully\n", fileName);
            } else {
                printf("Failed to load file [%s]\n", fileName);
            }
        } else if (userInput == 'n') {
            menuCommand = new;
            write(commandPipe[1], &menuCommand, sizeof(menuCommand));
            printf("Enter a size for a new board...\n");
            scanf("%d", &newSize);
            write(commandPipe[1], &newSize, sizeof(newSize));
            read(dataPipe[0], &success, sizeof(success));
            if (success == 1) {
                printf("New game of size [%d] was successful\n", newSize);
            } else {
                printf("Failed to make new game of size [%d], resuming game in progress\n", newSize);
            }
        } else if (userInput == 'm') {
            menuCommand = move;
            write(commandPipe[1], &menuCommand, sizeof(menuCommand));
            printf("Enter a tile value to move...\n");
            int tileToMove = 0;
            scanf("%d", &tileToMove);
            write(commandPipe[1], &tileToMove, sizeof(tileToMove));
            read(dataPipe[0], &success, sizeof(success));
            if (success == 1) {
                printf("Tile [%d] has been moved\n", tileToMove);
            } else {
                printf("Failed to move tile [%d]\n", tileToMove);
            }
        } else {
            menuCommand = noAction; //keep pipes in sync when given bad input
            write(commandPipe[1], &menuCommand, sizeof(menuCommand));
            printf("Enter a valid command...\n");
        }
    }
}