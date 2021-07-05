
#include <curses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
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

void initGame();
void getKey();
void moveSnake(int, siginfo_t *, void *);
void drawSnake(struct segment *);
int isSnakeCollision(int, int, int, int);
int addSegments(struct segment *, const int, int, int);
void spawnTrophy();
void displayResults(WINDOW *, char *, int, int);
void setTimer(long);
void quit();

const int MIN_WIDTH = 10;           // The minimum number of columns the snake pit can be
const int MIN_HEIGHT = 10;          // The minimum number of rows the snake pit can be
const long MOVE_BASE = 2000;        // Used to caculate how fast the snake will move

int isRunning = true;       	    // Set to false to quit the game

WINDOW *window;
int pitWidth;               	    // Width of the snake pit including the borders
int pitHeight;              	    // Height of the snake pit including the borders

struct segment *snake;
int maxLength;              	    // The longest the snake can be and the condition for winning
int snakeSize;              	    // Current size of the snake

int trophyVal;              	    // The number of segments added from the trophy
int trophyX;                	    // X position of the trophy
int trophyY;                	    // Y position of the trophy

int main() {
    // Setup the screen
    window = initscr();
    clear();
    noecho();
    cbreak();
    curs_set(false);
    keypad(window, true);
    srand(time(NULL));

    // The screen size must be a minimum of MIN_HEIGHT x MIN_WIDTH to play
    if(LINES < MIN_WIDTH || COLS < MIN_HEIGHT) {
        endwin();
        fprintf(stderr, "The window must be at least %d rows x %d columns\n", MIN_HEIGHT, MIN_WIDTH);
        return 1;
    }

    // Initialize the game and wait for user input
    initGame();
    while(isRunning) {
        getKey();
    }

    quit();
    return 0;
}

// Author: Bruno Francisco
// Initializes the game to the default settings at the beggining of the game
void initGame() {
    // Clear the screen
    clear();

    // Snake will move after a certain period of time has passed
    struct sigaction moveSignal;
    moveSignal.sa_sigaction = moveSnake;
    moveSignal.sa_flags = SA_SIGINFO;
    sigaction(SIGALRM, &moveSignal, NULL);

    // Snake pit takes up the entire screen size
    pitWidth = COLS;
    pitHeight = LINES;

    // Draw a border around the snake pit
    box(window, 0, 0);

    // Initialize the snake
    maxLength = pitWidth + pitHeight;
    snake = malloc(sizeof(*snake) * maxLength);
    snake[0].x = pitWidth / 2;
    snake[0].y = pitHeight / 2;

    // The initial direction of the snake is chosen at random
    int dir = rand() % 4;
    switch(dir) {
        case 0:                         // UP
            snake[0].xDir = 0;
            snake[0].yDir = -1;
            break;
        case 1:                         // DOWN
            snake[0].xDir = 0;
            snake[0].yDir = 1;
            break;
        case 2:                         // LEFT
            snake[0].xDir = -1;
            snake[0].yDir = 0;
            break;
        case 3:                         // RIGHT
            snake[0].xDir = 1;
            snake[0].yDir = 0;
            break;
    }

    // Snake has an initial length of 'initSize'
    int initSize = 3;
    snakeSize = addSegments(snake, maxLength, 1, initSize);
    drawSnake(snake);
    move(1, 1);
    printf("%d\n", snakeSize);

    // Create the first trophy
    spawnTrophy();

    // Draw the screen
    wrefresh(window);

    // Determine how fast the snake will move
    // Snake speed depends on its size
    long moveRate = MOVE_BASE / snakeSize;
    setTimer(moveRate);
}

// Author: Bruno Francisco
// Gets user input and respond as follows:
// If the user presses 'q' or 'Q' or an EOF is sent, the game quits
// If the user presses one of the arrow keys or the 'w', 'a', 's', or 'd' keys,
// the snakes direction is changed to reflect the arrow key pressed
void getKey() {
    int key = getch();
    switch(key) {
        case 'Q':                       // 'q', 'Q', or EOF --> quit game
        case 'q':
            isRunning = false;
            break;
        case 'w':                       // 'w' key or
        case KEY_UP:                    // up arrow key --> snake direction is set to up
            snake[0].xDir = 0;
            snake[0].yDir = -1;
            break;
        case 's':                       // 's' key or
        case KEY_DOWN:                  // down arrow key --> snake direction is set to down
            snake[0].xDir = 0;
            snake[0].yDir = 1;
            break;
        case 'a':                       // 'a' key or
        case KEY_LEFT:                  // left arrow key --> snake direction is set to left
            snake[0].xDir = -1;
            snake[0].yDir = 0;
            break;
        case 'd':                       // 'd' key or
        case KEY_RIGHT:                 // right arrow key --> snake direction is set to right
            snake[0].xDir = 1;
            snake[0].yDir = 0;
            break;
    }
}

// Author: Bruno Francisco
// Moves the snake in the currently set direction
// and checks for collision between the borders and snake body
// If collsion is detected, print a game over screen
void moveSnake(int sig, siginfo_t *info, void *ucontext) {
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

    // Check for collision with trophy
    // Add 'trophyVal' segments to the snake if their is a collision
    // If the number of segments is equal to the maximum length the user wins
    if(snake[0].x == trophyX && snake[0].y == trophyY) {
        snakeSize = addSegments(snake, maxLength, snakeSize, trophyVal);
        if(snakeSize == maxLength) {
            displayResults(window, "YOU WIN!", pitWidth, pitHeight);
            quit();
            exit(0);
        }
        spawnTrophy();
        setTimer(MOVE_BASE / snakeSize);
    }

    // Draw the snake
    drawSnake(snake);

    // Check to see if the snake head has collided with the body
    if(isSnakeCollision(snake[0].x, snake[0].y, 1, snakeSize)) {
        displayResults(window, "GAME OVER", pitWidth, pitHeight);
        quit();
        exit(0);
    }

    // Check to see if the snake head has collided with the pit walls
    if(snake[0].x <= 0 || snake[0].x >= pitWidth - 1 ||
       snake[0].y <= 0 || snake[0].y >= pitHeight - 1) {
        displayResults(window, "GAME OVER", pitWidth, pitHeight);
        quit();
        exit(0);
    }

    // Place cursor in a non-intrusive place and update the screen
    move(pitHeight - 1, pitWidth - 1);
    wrefresh(window);
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
// Checks if the the coordinate (x, y) lies within
// the snake segments between indices [start, end)
int isSnakeCollision(int x, int y, int start, int end) {
    for(int i = start; i < end; i++)
        if(x == snake[i].x && y == snake[i].y)
            return true;
    return false;
}

// Author: Bruno Francisco
// Appends to the end of the snake at most 'segNum' amount of segments
// and returns the new length of the snake
int addSegments(struct segment *snake, int maxLength, int size, int segNum) {
    for(int i = 0; i < segNum && size < maxLength; i++) {
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
// Creates a new trophy in a random location not inside the snake
// from 'x' to 'width' and 'y' to 'height'
// but does not refresh the screen
void spawnTrophy() {
    do {
        trophyVal = rand() % 9 + 1;                 // Trophy has a value between 1 and 9 inclusive
        trophyX = rand() % (pitWidth - 2) + 1;      // X position of trophy
        trophyY = rand() % (pitHeight - 2) + 1;     // Y position of trophy
    } while(isSnakeCollision(trophyX, trophyY, 0, snakeSize));

    int digitChar = '0' + trophyVal;            // Draw the character representation of the
    move(trophyY, trophyX);                     // trophies value
    addch(digitChar);
    move(pitHeight, pitWidth);
}

// Author: Bruno Francisco
// Clears the screen and displays 'message' at the center of the snake pit
// Allows the user to choose between resarting the game or quiting
void displayResults(WINDOW *win, char *message, int width, int height) {
    // Clear the screen and draw the border again
    wclear(win);
    box(win, 0, 0);

    // Put 'message' in the center of the screen
    int centerX = width / 2;
    int centerY = height / 2;
    int messageLen = strlen(message);
    move(centerY, centerX - (messageLen / 2));
    addstr(message);

    // Put the cursor out of the way and update the screen
    move(width - 1, height - 1);
    wrefresh(win);

    // Wait for 'waitTime' microseconds
    long waitTime = 5000000;
    usleep(waitTime);
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

// Free any dynamically allocated space and restore the screen
// and set variables that keep the game running to false
void quit() {
    free(snake);
    endwin();
    isRunning = false;
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
