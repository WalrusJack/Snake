
#include <curses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

// Author: Bruno Francsico
// Stores the position (x, y) and directon (xDir, yDir)
// of a single segment of the larger snake
struct segment {
    int x;
    int y;
    int xDir;               // xDir = 1 --> right, xDir = -1 --> left
    int yDir;               // yDir = 1 --> down, yDir = -1 --> up
};

void moveSnake(int);
void drawSnake(struct segment *);
int addSegments(struct segment *, const int, int, int);
void setTimer(long);

WINDOW *window;
int pitWidth;               // Width of the snake pit including the borders
int pitHeight;              // Height of the snake pit including the borders

struct segment *snake;
int maxLength;              // The longest the snake can be and the condition for winning
int snakeSize;              // Current size of the snake

long moveBase = 2000;       // Used to caculate how fast the snake will move

int main() {
    // Author: Bruno Francisco
    // Setup the screen
    window = initscr();
    clear();
    noecho();
    cbreak();
    curs_set(false);
    keypad(window, true);
    
    // Snake will move after a certain period of time has passed
    signal(SIGALRM, moveSnake);

    // Snake pit takes up the entire screen size
    pitWidth = COLS;
    pitHeight = LINES;

    // drawBorder(pitWidth, pitHeight);
    box(window, 0, 0);

    // Initialize the snake
    maxLength = pitWidth + pitHeight;
    snake = malloc(sizeof(*snake) * maxLength);
    snake[0].x = pitWidth / 2;
    snake[0].y = pitHeight / 2;
    snake[0].xDir = 1;
    snake[0].yDir = 0;
    int initSize = 5;
    snakeSize = addSegments(snake, maxLength, 1, initSize);
    drawSnake(snake);
    
    // Draw the screen
    refresh();
    
    // Determine how fast the snake will move
    // Snake speed depends on its size
    long moveRate = moveBase / snakeSize;
    setTimer(moveRate);
    
    // Author: Bruno Francisco
    // Check for user input
    // If the user presses 'q' or 'Q' the game quits
    // If the user presses one of the arrow keys, the snakes
    // direction is changed to reflect the arrow key pressed
    int quit = false;
    while(!quit) {
        int key = getch();
        switch(key) {
            case 'Q':                       // 'q' or 'Q' --> quit game
            case 'q':
                quit = true;
                break;
            case KEY_UP:                    // up arrow key --> snake direction is set to up
                snake[0].xDir = 0;
                snake[0].yDir = -1;
                break;
            case KEY_DOWN:                  // down arrow key --> snake direction is set to down
                snake[0].xDir = 0;
                snake[0].yDir = 1;
                break;
            case KEY_LEFT:                  // left arrow key --> snake direction is set to left
                snake[0].xDir = -1;
                snake[0].yDir = 0;
                break;
            case KEY_RIGHT:                 // right arrow key --> snake direction is set to right
                snake[0].xDir = 1;
                snake[0].yDir = 0;
                break;
        }
    }
    
    free(snake);        // Free the space taken up by the snake array
    endwin();           // Restore the screen and quit the game
    
    return 0;
}

// Author: Bruno Francisco
// Moves the snake in the currently set direction
// and checks for collision between the borders and snake body
// If collsion is detected, print a game over screen
void moveSnake(int signum) {
    // Move the head of the snake into its new position
    snake[0].x += snake[0].xDir;
    snake[0].y += snake[0].yDir;

    // Update the position and direction of each segment in the snake
    for(int i = snakeSize - 1; i > 0; i--) {
        snake[i].x += snake[i].xDir;
        snake[i].y += snake[i].yDir;
        snake[i].xDir = snake[i - 1].xDir;
        snake[i].yDir = snake[i - 1].yDir;
    }
    
    // Draw the snake
    drawSnake(snake);
    
    // Check to see if the snake head has collided with the body
    for(int i = 1; i < snakeSize; i++)                      
        if(snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            free(snake);
            endwin();
            exit(0);
        }
    
    // Check to see if the snake head has collided with the pit walls
    if(snake[0].x <= 0 || snake[0].x >= pitWidth - 1 || snake[0].y <= 0 || snake[0].y >= pitHeight - 1) {
        free(snake);
        endwin();
        exit(0);
    }

    // Place cursor in a non-intrusive place and update the screen
    move(pitHeight - 1, pitWidth - 1);
    refresh();
}

// Bruno Francisco
// Draws the snake on the screen but does not refresh it
void drawSnake(struct segment *snake) {
    // Characters to be used to display the head and body of the snake
    const int headChar = '#';
    const int bodyChar = '#';

    // Draw the head
    move(snake[0].y, snake[0].x);
    addch(headChar);

    // Draw the body
    for(int i = 1; i < snakeSize - 1; i++) {
        move(snake[i].y, snake[i].x);
        addch(bodyChar);
    }

    // Delete the old tail of the snake
    move(snake[snakeSize - 1].y, snake[snakeSize - 1].x);
    addch(' ');
    
    // Place the cursor in a non-invasive place
    move(pitHeight - 1, pitWidth - 1);
}

// Author: Bruno Francisco
// Appends to the end of the snake at most 'segNum' amount of segments
// and returns the new length of the snake
int addSegments(struct segment *snake, int maxLength, int size, int segNum) {
    for(int i = 0; i < segNum && size < maxLength - 1; i++) {
        int prev = size - 1;
        snake[size].x = snake[prev].x - snake[prev].xDir;       // The current segments position and
        snake[size].y = snake[prev].y - snake[prev].yDir;       // direction is set to the previous
        snake[size].xDir = snake[prev].xDir;                    // segments position and direction
        snake[size].yDir = snake[prev].yDir;
        size++;                                                 // Increase the length of snake
    }
    return size;
}

// Author: Bruno Francisco
// Sets a repeating timer of 'time' milliseconds
// Used to move the snake in after a certain period of time passes
void setTimer(long time) {
    struct itimerval timer;
    long sec = time / 1000;                     // Number of seconds in 'time'
    long usec = (time % 1000) * 1000L;          // Number of microseconds in 'time' excluding seconds

    timer.it_interval.tv_sec = sec;             // Timer will start over after it has finished
    timer.it_interval.tv_usec = usec;
    timer.it_value.tv_sec = sec;
    timer.it_value.tv_usec = usec;

    setitimer(ITIMER_REAL, &timer, NULL);       // Set the timer
}

/* ::UNUSED::
// Author: Kobe Onye
// Draws the border and the background for the snake to move in
void border(int height, int width)
{
    int xMax, yMax;
    getmaxyx(stdscr, yMax, xMax);
    WINDOW *board_win = newwin(height, width, (yMax/2) - (height/2), (xMax/2) - (width/2)); // Creates the border 
    box(board_win, 0, 0);                                                                   // Draws a straight line to be used as the snake border
    wrefresh(board_win);
}*/
