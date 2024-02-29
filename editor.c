#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define MAX_ROWS 100
#define MAX_COLS 100

char buffer[MAX_ROWS];

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
    }
}

// Function to delete character at cursor position
void deleteCharacter(char text[MAX_ROWS][MAX_COLS], int *cursorRow, int *cursorCol) {
    if (*cursorCol > 0) {
        moveCursorLeft(cursorRow, cursorCol);
        for (int i=*cursorCol;i<MAX_ROWS-1;i++){
    	    text[*cursorRow][i]=text[*cursorRow][i+1];   
    	}
        printf("\033[P"); // Delete character
    }
}

void specialCharacter(int *cursorRow, int *cursorCol, int c) {
    if (c == 20) {
    	printf("✔");
    }
    else if (c == 24) {
    	printf("X");
    }
    else {
    	putchar(c);
    }
}

// Function to insert character at cursor position
void insertCharacter(char text[MAX_ROWS][MAX_COLS], int *cursorRow, int *cursorCol, int c) {
    if (*cursorCol < MAX_COLS) {
        if (c=='\n') {
            int valid=0;
            for (int i=MAX_COLS-2;i>=0;i--){
            	if (text[*cursorRow][i]=='\n'){
            	    valid=1;
            	    break;
            	}
            	else if (text[*cursorRow][i]!=0){
            	    text[*cursorRow][i+1]='\n';
            	    valid=1;
            	    break;
            	}
            }
            if (valid==0) {
            	text[*cursorRow][0]='\n';
            }
            specialCharacter(cursorRow, cursorCol, c);
            moveCursorNext(cursorRow, cursorCol);
        }
        else {
            text[*cursorRow][*cursorCol] = c;
    	    specialCharacter(cursorRow, cursorCol, c);
            if (*cursorCol == MAX_COLS) {
   	        moveCursorNext(cursorRow, cursorCol);
   	    }
   	    else {
   	    	(*cursorCol)++;
   	    }
        }
    }
}

void saveCharacter(char c, FILE *file) {
    if (c != 0) {
    	fputc(c, file);
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
            		saveCharacter(text[i][j], file);
                }
            }
            fclose(file);
            break;
        }
        else if (c == 2) {
            //Copy button
            if (cursorCol==0)
            for (int i=0;i<MAX_COLS;i++){
            	buffer[i]=text[cursorRow][i];
            }
        }
        else if (c == 22) {
            //paste button
            if (cursorCol==0){
                for (int i=0;i<MAX_COLS;i++){
            	    if (buffer[i]!='\n') {
            	    	insertCharacter(text, &cursorRow, &cursorCol, buffer[i]);
            	    }
            	    else{
            	    	break;
            	    }
                }
            }
        }
        else if (c == 27) { // Escape key for cursor movement
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
	}
        else {
            insertCharacter(text, &cursorRow, &cursorCol, c);
        }
    }

    resetTermios(&oldTermios); // Restore terminal settings
    printf("\nChanges saved to %s\n", filename);

    return 0;
}
