/*
 * Representing server side of "sliding puzzle" game
 * Uses C99 standard
 * Tested and compiled on Apple clang version 12.0.0 (clang-1200.0.32.28)
 * Tests run on x86_64-apple-darwin20.2.0
 * @author Jesse Clegg
 * @version 3.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

enum menuOptions {
    print, save, load, new, move, noAction
};

const int emptyTileValue = -1;
int **board = NULL;
int boardSize;
int rows;
int cols;
int maxTileValue;
int randomPool[99];
int gamesPlayed = 0;
int isLoadingGame = 0;
int location[2];//stores i and j location of a desired value found with searchBoard() at indices [0] and [1] respectively.

/*
 * A function to save a current game board into a file, creates new or overwrites to avoid conflicts with existing files
 * saved in %2d, as a valid board will never have 3 digit tile value, delimited by spaces for readability/testing.
 * @param fileName: this will be the name of your save file created if successful.
 * @return: 0 if the file fails to save, 1 if successful.
 */
int saveGame(char *fileName) {
    int wasSuccessful = 0;
    FILE *filePtr;
    filePtr = fopen(fileName, "w");
    if (filePtr == NULL) {
        wasSuccessful = 0;
        return wasSuccessful;
    } else {
        wasSuccessful = 1;
        int currentTileValue;
        for (int i = 0; i < boardSize; i++) {
            for (int j = 0; j < boardSize; j++) {
                currentTileValue = (board[i])[j];
                fprintf(filePtr, "%2d ", currentTileValue);
            }
        }
        fclose(filePtr);
        return wasSuccessful;
    }
}

/*
 * Populates a global array 'randomPool' filled with each tile value required for the new board being constructed.
 * The [0] index is always the empty tile value, each value is the same as its index location,
 * because of this empty tile offset.
 * Given a fixed upper bound of tiles, and the way the pool will be accessed, there is no need to wipe any old values
 * that are greater than the upper bounds of the new board being currently constructed.
 * To do so would be computationally wasteful
 */
void fillRandomPool() {
    srand(time(NULL));
    randomPool[0] = -1;
    for (int i = 1; i < (maxTileValue + 1); i++) {
        randomPool[i] = i;
    }
}

/*
 * A random index location in the global array is selected and returned.
 * Using '% (maxTileValue + 1)' forces this index location to fall within the bounds of possible tile values for the current board.
 * Once an index has been selected and the value read, that index is set to 0 to prevent duplicate values being returned.
 * @return the integer value stored at the random index in global array 'randomPool'.
 */
int retrieveRandomValue() {
    int valueFound;
    int index = ((rand()) % (maxTileValue + 1));
    valueFound = randomPool[index];
    if (valueFound == 0) {
        return valueFound;
    } else {
        randomPool[index] = 0;
        return valueFound;
    }
}

/*
 * Allows for direct access of a single tile on the board, makes for cleaner access when swapping.
 * @param i: the i location of this element to be changed.
 * @param j: the j location of this element to be changed.
 * @param value: this is the value that tile located at i,j will be changed to.
 */
void setOneTile(int i, int j, int value) {
    (board[i])[j] = value;
}

/*
 * Randomly assigns appropriate values to a game board scaled by global variable 'boardSize'.
 * Walks each "index" of this board and at every location, a call is made to retrieveRandomValue(), which returns
 * a random value, or 0. 0's are rejected, and the call is repeated until a nonzero value is returned.
 * That nonzero value is then assigned to the current index i,j.
 * This must be done to prevent repeat values.
 * Then we advance to the next index and repeat until all indices are filled.
 */
void setAllTiles() {
    fillRandomPool();
    int candidate = 1;
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            do {
                candidate = retrieveRandomValue();
            } while (candidate == 0);
            (board[i])[j] = candidate;
        }
    }
}

/*
 * Sets and scales dimensions for the board in relation to a newSize passed in.
 * Necessary to allow all other functions to operate with the scale of this new size.
 * @param newSize: this is the size of the new board that all values will be set in relation to.
 */
void setBoardSizeAndValues(int newSize) {
    boardSize = newSize;
    rows = boardSize;
    cols = boardSize;
    maxTileValue = (boardSize * boardSize) - 1;
}

/*
 * Frees all memory of the current board.
 * Must walk the outer array of pointers in order to free each pointer contained inside.
 */
void freeBoardMemory() {
    for (int i = 0; i < rows; i++) {
        free(*(board + i));
    }
    free(board);
}

/*
 * Ends the current game, informs the user.
 * Must call freeBoardMemory() in order to free up dynamically allocated memory allotted to the game we are ending.
 */
void tearDown() {
    freeBoardMemory();
}

/*
 * Dynamically allocates memory for a board in relation to global 'boardSize'.
 * Must be dynamically allocated to scale in demand of unknown board sizes.
 * Here we do the hard work up front with the allocation of an array of pointers, this provides easier access later on.
 */
void allocateMemory() {
    board = (int **) malloc(rows * sizeof(int *));
    for (int i = 0; i < rows; i++) {
        *(board + i) = (int *) malloc(cols * sizeof(int));
    }
}

/*
 * Starts a new game of size specified by input param.
 * Reject boards greater than 9, or less than 3, "between 2-10" interpreted as 2-10 non-inclusive.
 * Frees up previously allocated memory if applicable, and offers a different prompt whether call is for:
 *  -The first game played for this session
 *  -New game being created
 *  -Loading an existing game
 * Global board dimensions are established in relation to this new board, memory is allocated,
 * and appropriate values are assigned whether loading or randomizing a new game.
 * Decisions made within the function allow reuse of the same initialization() function in all applicable situations.
 * @param sizeOfNewBoard: this is the size of the new board that all above operations will be in relation to.
 */
int initialize(int sizeOfNewBoard) {
    if (sizeOfNewBoard > 9 || sizeOfNewBoard < 3) {
        return 0;
    }
    tearDown();
    setBoardSizeAndValues(sizeOfNewBoard);
    allocateMemory();
    if (isLoadingGame != 1) {
        setAllTiles();
    }
    isLoadingGame = 0;
    gamesPlayed++;
    return 1;
}

/*
 * A function to determine the number of tiles in a board to be loaded.
 * This information is needed to see if we reject a board to load or not.
 * Capable of reading any reasonable AND COMPATIBLE board file because of dynamic allocation.
 * @param fileName: This file name will be looked for in the current directory, then loaded if possible.
 * @return: 0 if the file is not present, else return the number of tiles in this file.
 */
int loadBoardSize(char *fileName) {
    int sizeTracker = 0;
    char *tempArray;
    tempArray = malloc(999 * sizeof(int));
    FILE *filePtr = fopen(fileName, "r");
    if (filePtr == NULL) {
        sizeTracker = -1;
        fclose(filePtr);
        free(tempArray);
        return sizeTracker;
    } else {
        while (!feof(filePtr)) {
            fscanf(filePtr, "%s", tempArray);
            sizeTracker++;
        }
        fclose(filePtr);
        free(tempArray);//Releases memory after closing the file.
        sizeTracker--; //SizeTracker result is one bigger than the actual number, so we decrement.
        return sizeTracker;
    }
}

/*
 * Evaluate a given fileName to see if compatible.
 * Must check if the game is a valid size.
 * If file is valid, initialize a new game with the board size needed, then load all values into board.
 * Must set global flag isLoadingGame to true, so we do not fill with random values.
 * Resumes current game if loadGame() fails.
 * @return: 1 for successful loading, 0 if error.
 */
int loadGame(char *fileName) {
    int wasSuccessful = 0;
    int size = loadBoardSize(fileName);
    if (size == -1) {
        wasSuccessful = 0;
        return wasSuccessful;
    } else {
        wasSuccessful = 1;
        FILE *filePtr;
        filePtr = fopen(fileName, "r");
        size = (int) sqrt(size);
        isLoadingGame = 1;
        initialize(size);
        for (int i = 0; i < boardSize; i++) {
            for (int j = 0; j < boardSize; j++) {
                fscanf(filePtr, "%3d", &(board[i])[j]);
            }
        }
        fclose(filePtr);
        return wasSuccessful;
    }
}

/*
 * Function that walks each tile in the board and compares that tile to a desired value.
 * If value is found, the i and j locations are stored in global location array, indices [0] and [1] respectively.
 * @param desiredValue: every tile on board will be checked against this value.
 */
void searchBoard(int desiredValue) {
    int currentTile = 0;
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            currentTile = (board[i])[j];
            if (currentTile == desiredValue) {
                location[0] = i;
                location[1] = j;
                return;
            }
        }
    }
}

/*
 * Evaluates if a given tile can be moved based its location in relation to the empty tile.
 * Limits possible tiles to evaluate based on the tile values currently present on the board.
 * searchBoard() is critical to retrieve the i and j location of tile values, as movement is determined by the tile.
 * position represented by those i and j values which are stored in the global array 'location' after each search.
 * Only need two checks: up/down and left/right, this is more efficient than checking all 4 directions separately.
 * Does not follow a typical cartesian plane, but the i and j could be understood as x and y if that aids in reading:
 *  - i = y coordinate
 *  - j = x coordinate
 * @param tileToCheck: this value will be located if on the board, and checked if it is located next to the empty tile.
 * @return: 1 if the tile to check is eligible to move, 0 if the move is not valid.
 */
int isMoveValid(int tileToCheck) {
    int valid = 0;
    if (tileToCheck < 1 || tileToCheck > maxTileValue) {
        return valid;
    } else {
        searchBoard(tileToCheck);
        int tileToMoveI = location[0];
        int tileToMoveJ = location[1];
        searchBoard(emptyTileValue);
        int emptyTileI = location[0];
        int emptyTileJ = location[1];
        if ((tileToMoveI == (emptyTileI + 1) || tileToMoveI == (emptyTileI - 1)) &&
            emptyTileJ == tileToMoveJ) {//Tile is above or below
            valid = 1;
            return valid;
        }
        if ((tileToMoveJ == (emptyTileJ + 1) || tileToMoveJ == (emptyTileJ - 1)) &&
            emptyTileI == tileToMoveI) {//Tile is left or right
            valid = 1;
            return valid;
        } else {
            valid = 0;
            return valid;
        }
    }
}

/*
 * Prompts for a tile to move
 * Must make use of isMoveValid() to determine whether to perform a swap, no internal evaluation.
 * Rejects bad moves and continues game, performs valid moves by swapping the tile values.
 * Relies on setOneTile() for the actual swap as there is no internal swapping functionality.
 * No explicit need for a success flag 'wasMoved', but one is included for future iterations/versions.
 * @return: 1 on success, 0 on a rejected move.
 */
int moveTile(int desiredValue) {
    int wasMoved = 0;
    if (isMoveValid(desiredValue)) {
        int temp[2];
        searchBoard(desiredValue);
        temp[0] = location[0];
        temp[1] = location[1];
        searchBoard(emptyTileValue);
        setOneTile(location[0], location[1], desiredValue);
        setOneTile(temp[0], temp[1], emptyTileValue);
        wasMoved = 1;
        return wasMoved;
    } else {
        return wasMoved;
    }
}


/*
 * Starts with index of 0,0 and walks board in appropriate order.
 * As long as next tile is one greater than current.
 * or next tile is the empty tile, game is won.
 * Needs to consider empty tile in any possible location, so we use the logical || when walking board.
 * @return: 1 on a winning board, else return 0.
 */
int isWon() {
    int index = 0;
    int currentTileValue;
    int expectedValue = (boardSize * boardSize) - 1;
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            currentTileValue = (board[i])[j];
            index++;
            if (currentTileValue == expectedValue || currentTileValue == -1) {// -1 is empty tile representation
                if (currentTileValue != -1) {
                    expectedValue--;
                }
            } else {
                return 0;
            }
        }
    }
    return 1;
}

/*
 * Server side receives commands from client, performs all computations and returns the results via pipes
 * Straightforward design pattern any reasonable developer should know
 * @param: command is the command pipe created in main function, accessible to both client and server
 * @param: data is the data pipe created in main function, accessible to both client and server
 */
void serverFunction(int *command, int *data) {
    int commandPipe[2];
    commandPipe[0] = *command;
    commandPipe[1] = *(command + 1);
    int dataPipe[2];
    dataPipe[0] = *data;
    dataPipe[1] = *(data + 1);
    close(commandPipe[1]);
    close(dataPipe[0]);
    enum menuOptions recieved;
    int winningGame;
    int hasQuit;
    int successful;
    int tileToMove;
    int newSize;
    char fileName[99];

    initialize(4);
    while (1) {
        winningGame = isWon();
        write(dataPipe[1], &winningGame, sizeof(int));
        if (winningGame == 1) {
            initialize(4);
        }
        hasQuit = read(commandPipe[0], &recieved, sizeof(recieved));
        if (hasQuit == 0) {
            break;
        }
        if (recieved == print) {
            write(dataPipe[1], &boardSize, sizeof(boardSize));
            for (int i = 0; i < boardSize; i++) {
                for (int j = 0; j < boardSize; j++) {
                    int currentTileValue;
                    currentTileValue = (board[i])[j];
                    write(dataPipe[1], &currentTileValue,
                          sizeof(int));//write each element over to client, perfectly synced
                }
            }
        } else if (recieved == save) {
            read(commandPipe[0], &fileName, sizeof(fileName));
            successful = saveGame(fileName);
            write(dataPipe[1], &successful, sizeof(successful));
        } else if (recieved == load) {
            read(commandPipe[0], &fileName, sizeof(fileName));
            successful = loadGame(fileName);
            write(dataPipe[1], &successful, sizeof(successful));
        } else if (recieved == new) {
            read(commandPipe[0], &newSize, sizeof(newSize));
            successful = initialize(newSize);;
            write(dataPipe[1], &successful, sizeof(successful));
        } else if (recieved == move) {
            read(commandPipe[0], &tileToMove, sizeof(tileToMove));
            successful = moveTile(tileToMove);
            write(dataPipe[1], &successful, sizeof(successful));
        } else if (recieved == noAction) {
            continue;
        }
    }
    tearDown();
    close(commandPipe[0]);
    close(dataPipe[1]);
}