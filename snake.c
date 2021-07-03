
#include <curses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// Author: Bruno Francsico
// Stores the position (x, y) and directon (xDir, yDir)
// of a single segment of the larger snake
struct segment {
    int x;
    int y;
    int xDir;           // xDir = 1 --> right, xDir = -1 --> left
    int yDir;           // yDir = 1 --> down, yDir = -1 --> up
};

WINDOW *window;
int pitWidth;
int pitHeight;

struct segment *snake;
int maxLength;
int snakeSize;

// Author: Bruno Francisco
// Moves the snake in the currently set direction
// and checks for collision between the borders and snake body
// If collsion is detected, print a game over screen
void moveSnake(int signum) {
    // Delete the old tail of the snake
    move(snake[snakeSize - 1].y, snake[snakeSize - 1].x);
    addch(' ');

    // Move the head of the snake into its new position
    snake[0].x += snake[0].xDir;
    snake[0].y += snake[0].yDir;
    move(snake[0].y, snake[0].x);
    addch('#');

    // Update the position and direction of each segment in the snake
    for(int i = snakeSize - 1; i > 0; i--) {
        snake[i].x += snake[i].xDir;
        snake[i].y += snake[i].yDir;
        snake[i].xDir = snake[i - 1].xDir;
        snake[i].yDir = snake[i - 1].yDir;
        move(snake[i].y, snake[i].x);
        addch('#');
    }

    // Check to see if the snake head has collided with the body
    for(int i = 1; i < snakeSize; i++)                      
        if(snake[0].x == snake[i].x && snake[0].y == snake[i].y)
            exit(0);
    
    // Check to see if the snake head has collided with the pit walls
    if(snake[0].x <= 0 || snake[0].x >= pitWidth - 1 || snake[0].y <= 0 || snake[0].y >= pitHeight - 1)
        exit(0)

    // Place cursor in a non-intrusive place and update the screen
    move(pitHeight - 1, pitWidth - 1);
    refresh();
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

// Author: Kobe Onye
// Draws the border and the background for the snake to move in
void border(int height, int width)
{
    int xMax, yMax;
    getmaxyx(stdscr, yMax, xMax);
    WINDOW *board_win = newwin(height, width, (yMax/2) - (height/2), (xMax/2) - (width/2)); // Creates the border 
    box(board_win, 0, 0);                                                                   // Draws a straight line to be used as the snake border
    wrefresh(board_win);
}
