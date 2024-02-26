#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define MAX_ROWS 100
#define MAX_COLS 100

// Function to turn off canonical mode and echo in terminal
void initTermios(struct termios *oldTermios) {
    struct termios newTermios;
    tcgetattr(0, oldTermios); // 0 is the file descriptor for stdin
    newTermios = *oldTermios;
    newTermios.c_lflag &= ~(ICANON | ECHO); // Turn off canonical mode and echo
    tcsetattr(0, TCSANOW, &newTermios);
}

// Function to restore terminal settings
void resetTermios(struct termios *oldTermios) {
    tcsetattr(0, TCSANOW, oldTermios);
}

// Function to move cursor left
void moveCursorLeft(int *cursorRow, int *cursorCol) {
    if (*cursorCol > 0) {
        printf("\033[D"); // Move cursor left
        (*cursorCol)--;
    }
}

// Function to move cursor right
void moveCursorRight(int *cursorRow, int *cursorCol) {
    if (*cursorCol < MAX_COLS ) {
    	printf("\033[C"); // Move cursor right
    	(*cursorCol)++;
   }
}

// Function to move cursor up
void moveCursorUp(int *cursorRow, int *cursorCol) {
    if (*cursorRow > 0) {
        printf("\033[A"); // Move cursor up
        (*cursorRow)--;
    }
}

// Function to move cursor down
void moveCursorDown(int *cursorRow, int *cursorCol) {
    if (*cursorRow < MAX_ROWS - 1){
    	printf("\033[B"); // Move cursor down
    	(*cursorRow)++;
    }
}

// Function to move cursor next line
void moveCursorNext(int *cursorRow, int *cursorCol) {
    if (*cursorRow < MAX_COLS - 1) {
    	(*cursorRow)++;
    	*cursorCol=0;
    	printf("\033[E");
    }
}

// Function to delete character at cursor position
void deleteCharacter(char text[MAX_ROWS][MAX_COLS], int *cursorRow, int *cursorCol) {
    if (*cursorCol > 0) {
        moveCursorLeft(cursorRow, cursorCol);
        text[*cursorRow][*cursorCol]=0;
        printf("\033[P"); // Delete character
    }
}

// Function to insert character at cursor position
void insertCharacter(char text[MAX_ROWS][MAX_COLS], int *cursorRow, int *cursorCol, char c) {
    if (*cursorCol < MAX_COLS) {
        text[*cursorRow][*cursorCol] = c;
        putchar(c);
        if (c=='\n') {
            moveCursorNext(cursorRow, cursorCol);
        }
        else {
            if (*cursorCol == MAX_COLS) {
   	        moveCursorNext(cursorRow, cursorCol);
   	    }
   	    else {
   	    	(*cursorCol)++;
   	    }
        }
    }
}

int main(int argc, char *argv[]) {
    struct termios oldTermios;
    char text[MAX_ROWS][MAX_COLS];
    char *filename;
    int c;
    int cursorRow = 0;
    int cursorCol = 0;
    
    for (int i = 0; i < MAX_ROWS; i++) {
    	for (int j = 0; j < MAX_COLS; j++) {
        	text[i][j]=0;	
        }
    }

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    filename = argv[1];

    FILE *file = fopen(filename, "r"); // Open file in read mode
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // Read existing content of the file
    while ((c = fgetc(file)) != EOF) {
        insertCharacter(text, &cursorRow, &cursorCol, c);
    }
    fclose(file);
    initTermios(&oldTermios); // Turn off canonical mode and echo

    while ((c = getchar()) != EOF) {
        if (c == 4) { // Ctrl+D
            file = fopen(filename, "w"); // Open file in write mode to overwrite its content
            if (!file) {
                fprintf(stderr, "Error opening file %s\n", filename);
                exit(EXIT_FAILURE);
            }
            // Write modified text to the file
            for (int i = 0; i < MAX_ROWS; i++) {
            	for (int j = 0; j < MAX_COLS; j++) {
            		if (text[i][j]!=0) {
                   		fputc(text[i][j], file);
                   	}
                }
            }
            fclose(file);
            break;
        } else if (c == 27) { // Escape key for cursor movement
            if ((c = getchar()) == '[') {
                switch (getchar()) {
                    case 'A': // Up arrow key
                        moveCursorUp(&cursorRow, &cursorCol);
                        break;
                    case 'B': // Down arrow key
                        moveCursorDown(&cursorRow, &cursorCol);
                        break;
                    case 'C': // Right arrow key
                        moveCursorRight(&cursorRow, &cursorCol);
                        break;
                    case 'D': // Left arrow key
                        moveCursorLeft(&cursorRow, &cursorCol);
                        break;
                }
            }
        } else if (c == 127) { // Backspace key
            deleteCharacter(text, &cursorRow, &cursorCol);
        } else {
            insertCharacter(text, &cursorRow, &cursorCol, c);
        }
    }

    resetTermios(&oldTermios); // Restore terminal settings
    printf("\nChanges saved to %s\n", filename);

    return 0;
}